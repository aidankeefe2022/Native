// Command testbench is a TLS server that returns a file (passed by argument)
// to any client that connects and asks for a program.
//
// It is the peer of src/native: native is a TCP+TLS *client* that connects
// here, sends a short request line ("asks for a program"), and reads the file
// bytes we send until we close the TLS session (close_notify). native stashes
// those bytes in a memfd, dlopen()s the result, and calls its `run` symbol.
//
// The server presents certs/server-cert.pem; native verifies it against
// certs/ca-cert.pem. No client certificate is requested.
package main

import (
	"bufio"
	"crypto/tls"
	"flag"
	"fmt"
	"log"
	"net"
	"os"
	"path/filepath"
	"strings"
	"time"
)

func main() {
	log.SetFlags(log.Ltime)

	// Anchor the default certs dir to the binary's location (repo root is the
	// parent of testBench/).
	defaultCerts := "certs"
	if exe, err := os.Executable(); err == nil {
		defaultCerts = filepath.Join(filepath.Dir(filepath.Dir(exe)), "certs")
	}

	addr := flag.String("addr", ":8000", "address to listen on")
	cert := flag.String("cert", filepath.Join(defaultCerts, "server-cert.pem"),
		"server certificate PEM presented during the handshake")
	key := flag.String("key", filepath.Join(defaultCerts, "server-key.pem"),
		"server private key PEM")

	flag.Usage = func() {
		fmt.Fprintf(os.Stderr, "usage: %s [flags] <file>\n\n", filepath.Base(os.Args[0]))
		fmt.Fprintf(os.Stderr, "Serves <file> over TLS 1.3 to clients that ask for a program.\n\n")
		flag.PrintDefaults()
	}
	flag.Parse()

	if flag.NArg() != 1 {
		flag.Usage()
		os.Exit(2)
	}
	filePath := flag.Arg(0)

	// Validate readability up front so misconfiguration fails loudly at start.
	if _, err := os.Stat(filePath); err != nil {
		log.Fatalf("cannot serve file %q: %v", filePath, err)
	}

	certPair, err := tls.LoadX509KeyPair(*cert, *key)
	if err != nil {
		log.Fatalf("cannot load cert/key: %v", err)
	}

	// Match the TLS 1.3 client method used by src/native.c.
	tlsCfg := &tls.Config{
		Certificates: []tls.Certificate{certPair},
		MinVersion:   tls.VersionTLS13,
		MaxVersion:   tls.VersionTLS13,
	}

	ln, err := tls.Listen("tcp", *addr, tlsCfg)
	if err != nil {
		log.Fatalf("cannot listen on %s: %v", *addr, err)
	}
	defer ln.Close()
	log.Printf("[*] serving %s over TLS 1.3 on %s", filePath, *addr)

	for {
		conn, err := ln.Accept()
		if err != nil {
			log.Printf("[!] accept failed: %v", err)
			continue
		}
		go handle(conn, filePath)
	}
}

func handle(conn net.Conn, filePath string) {
	defer conn.Close()
	peer := conn.RemoteAddr()

	tlsConn := conn.(*tls.Conn)
	// Bound the handshake + request read so a stalled client can't wedge a
	// goroutine forever.
	_ = tlsConn.SetDeadline(time.Now().Add(30 * time.Second))
	if err := tlsConn.Handshake(); err != nil {
		log.Printf("[!] %s: TLS handshake failed: %v", peer, err)
		return
	}

	// The client "asks for a program" with a single request line. We serve the
	// file we were configured with regardless of the requested name, but read
	// and log it so the exchange is explicit and the protocol can grow a real
	// program selector later.
	req, err := bufio.NewReader(tlsConn).ReadString('\n')
	if err != nil {
		log.Printf("[!] %s: reading request failed: %v", peer, err)
		return
	}
	log.Printf("[+] %s asked for %q", peer, strings.TrimSpace(req))

	// Read fresh each connection so a rebuilt file is picked up without a
	// restart.
	payload, err := os.ReadFile(filePath)
	if err != nil {
		log.Printf("[!] %s: cannot read %q: %v", peer, filePath, err)
		return
	}

	if _, err := tlsConn.Write(payload); err != nil {
		log.Printf("[!] %s: send failed: %v", peer, err)
		return
	}
	// Send close_notify so the client's read loop terminates cleanly.
	if err := tlsConn.CloseWrite(); err != nil {
		log.Printf("[!] %s: close_notify failed: %v", peer, err)
		return
	}
	log.Printf("[+] %s: sent %d bytes; transfer complete", peer, len(payload))
}
