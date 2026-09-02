#!/usr/bin/env python3
"""TLS client that streams a shared library to the `native` program.

Despite acting as the TLS *server* half of the handshake, this script is the
*connecting* side of the TCP connection -- it is a client for `src/native`.

That mirror image comes from how src/native.c is wired: the C program is a TCP
server (it bind()/listen()/accept()s on port 8000) but performs a TLS *client*
handshake with wolfSSL_connect().  So its peer must connect() to it over TCP
and then drive the TLS *server* side of the handshake before streaming bytes.

The C side reads bytes until we perform a clean TLS shutdown (wolfSSL_read
returns 0), stashes them in a memfd, then dlopen()s the result and calls `run`.
This client connects, sends the whole .so file, then sends close_notify so the
peer's read loop terminates cleanly.

The C program presents the wolfSSL example server cert/key but does not request
or verify a client certificate, so any cert this TLS server presents is
accepted; the defaults point at the certs/ tree that ships alongside it.
"""

import argparse
import os
import socket
import ssl
import sys
import time

_HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_LIB = "libAidan.so"
DEFAULT_CERTS = os.path.join(_HERE, "certs")


def parse_args():
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--host", default="127.0.0.1",
                   help="native's address to connect to (default 127.0.0.1)")
    p.add_argument("--port", type=int, default=8000,
                   help="native's port to connect to (default 8000)")
    p.add_argument("--lib", default=DEFAULT_LIB,
                   help="path to the shared library to serve (default %(default)s)")
    p.add_argument("--cert", default=f"{DEFAULT_CERTS}/server-cert.pem",
                   help="server certificate PEM presented during the handshake")
    p.add_argument("--key", default=f"{DEFAULT_CERTS}/server-key.pem",
                   help="server private key PEM")
    p.add_argument("--retries", type=int, default=0,
                   help="times to retry connecting if native isn't listening yet")
    p.add_argument("--retry-delay", type=float, default=0.5,
                   help="seconds between connection retries (default 0.5)")
    return p.parse_args()


def make_context(cert, key):
    ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    # Match the TLS 1.3 client method used by src/native.c.
    ctx.minimum_version = ssl.TLSVersion.TLSv1_3
    ctx.maximum_version = ssl.TLSVersion.TLSv1_3
    ctx.load_cert_chain(certfile=cert, keyfile=key)
    return ctx


def connect(host, port, retries, delay):
    last_err = None
    for attempt in range(retries + 1):
        try:
            conn = socket.create_connection((host, port))
            print(f"[+] connected to {host}:{port}")
            return conn
        except OSError as e:
            last_err = e
            if attempt < retries:
                print(f"[.] {host}:{port} not ready ({e}); retrying in {delay}s")
                time.sleep(delay)
    raise last_err


def serve(ctx, conn, payload):
    tls = None
    try:
        # native drives the TLS *client* side (wolfSSL_connect), so we are the
        # TLS server even though we opened the TCP connection.
        tls = ctx.wrap_socket(conn, server_side=True)
        print(f"[+] TLS handshake ok ({tls.version()}, {tls.cipher()[0]})")
        tls.sendall(payload)
        print(f"[+] sent {len(payload)} bytes")
        # Send close_notify so the peer's wolfSSL_read() returns 0 and its loop
        # ends.  native then frees the session without replying with its own
        # close_notify, so unwrap()'s wait for it hits EOF -- expected here, and
        # harmless since the payload is already delivered.
        try:
            tls.unwrap()
        except (ssl.SSLError, OSError):
            pass
        print("[+] sent close_notify; transfer complete")
    except ssl.SSLError as e:
        print(f"[!] TLS error: {e}", file=sys.stderr)
    except OSError as e:
        print(f"[!] socket error: {e}", file=sys.stderr)
    finally:
        try:
            (tls or conn).close()
        except OSError:
            pass


def main():
    args = parse_args()

    try:
        with open(args.lib, "rb") as f:
            payload = f.read()
    except OSError as e:
        sys.exit(f"cannot read library {args.lib!r}: {e}")

    try:
        ctx = make_context(args.cert, args.key)
    except (ssl.SSLError, OSError) as e:
        sys.exit(f"cannot load cert/key: {e}")

    print(f"[*] serving {args.lib} ({len(payload)} bytes) to "
          f"{args.host}:{args.port} over TLS 1.3")

    try:
        conn = connect(args.host, args.port, args.retries, args.retry_delay)
    except OSError as e:
        sys.exit(f"cannot connect to {args.host}:{args.port}: {e}")

    serve(ctx, conn, payload)


if __name__ == "__main__":
    main()
