/* Copyright (c) 2015, Google Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/pkcs8.h>
#include <openssl/x509.h>


/* kDER is a PKCS#8 encrypted private key. It was generated with:
 *
 * openssl genrsa 512 > test.key
 * openssl pkcs8 -topk8 -in test.key -out test.key.encrypted -v2 des3 -outform der
 * hexdump -Cv test.key.encrypted
 *
 * The password is "testing".
 */
static const uint8_t kDER[] = {
  0x30, 0x82, 0x01, 0x9e, 0x30, 0x40, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x05,
  0x0d, 0x30, 0x33, 0x30, 0x1b, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x05, 0x0c,
  0x30, 0x0e, 0x04, 0x08, 0x06, 0xa5, 0x4b, 0x0c, 0x0c, 0x50, 0x8c, 0x19, 0x02, 0x02, 0x08, 0x00,
  0x30, 0x14, 0x06, 0x08, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x03, 0x07, 0x04, 0x08, 0x3a, 0xd0,
  0x70, 0x4b, 0x26, 0x50, 0x13, 0x7b, 0x04, 0x82, 0x01, 0x58, 0xa6, 0xee, 0x02, 0xf2, 0xf2, 0x7c,
  0x19, 0x91, 0xe3, 0xce, 0x32, 0x85, 0xc5, 0x01, 0xd9, 0xe3, 0x5e, 0x14, 0xb6, 0xb8, 0x78, 0xad,
  0xda, 0x01, 0xec, 0x9e, 0x42, 0xe8, 0xbf, 0x0b, 0x46, 0x03, 0xbc, 0x92, 0x6f, 0xe4, 0x0f, 0x0f,
  0x48, 0x30, 0x10, 0x10, 0x9b, 0xfb, 0x4b, 0xb9, 0x45, 0xf8, 0xcf, 0xab, 0xa1, 0x18, 0xdd, 0x19,
  0xa4, 0xa4, 0xe1, 0xf0, 0xa1, 0x8d, 0xc2, 0x23, 0xe7, 0x0d, 0x7a, 0x64, 0x21, 0x6b, 0xfa, 0x48,
  0xb9, 0x41, 0xc1, 0x0c, 0x4b, 0xce, 0x6f, 0x1a, 0x91, 0x9b, 0x9f, 0xdd, 0xcf, 0xa9, 0x8d, 0x33,
  0x2c, 0x45, 0x81, 0x5c, 0x5e, 0x67, 0xc6, 0x68, 0x43, 0x62, 0xff, 0x5e, 0x9b, 0x1a, 0x15, 0x3a,
  0x9d, 0x71, 0x3f, 0xbe, 0x32, 0x2f, 0xe5, 0x90, 0x65, 0x65, 0x9c, 0x22, 0xf6, 0x29, 0x2e, 0xcf,
  0x26, 0x16, 0x7b, 0x66, 0x48, 0x55, 0xad, 0x9a, 0x8d, 0x89, 0xf4, 0x48, 0x4f, 0x1f, 0x9d, 0xb8,
  0xfa, 0xe1, 0xf1, 0x3b, 0x39, 0x5c, 0x72, 0xc6, 0xb8, 0x3e, 0x98, 0xe8, 0x77, 0xe8, 0xb6, 0x71,
  0x84, 0xa8, 0x6e, 0xca, 0xaf, 0x62, 0x96, 0x49, 0x8a, 0x21, 0x6f, 0x9e, 0x78, 0x07, 0x97, 0x38,
  0x40, 0x66, 0x42, 0x5a, 0x1b, 0xe0, 0x9b, 0xe9, 0x91, 0x82, 0xe4, 0xea, 0x8f, 0x2a, 0xb2, 0x80,
  0xce, 0xe8, 0x57, 0xd3, 0xac, 0x11, 0x9d, 0xb2, 0x39, 0x0f, 0xe1, 0xce, 0x18, 0x96, 0x38, 0xa1,
  0x19, 0x80, 0x88, 0x81, 0x3d, 0xda, 0xaa, 0x8e, 0x15, 0x27, 0x19, 0x73, 0x0c, 0xf3, 0xaf, 0x45,
  0xe9, 0x1b, 0xad, 0x6c, 0x3d, 0xbf, 0x95, 0xf7, 0xa0, 0x87, 0x0e, 0xde, 0xf1, 0xd8, 0xee, 0xaa,
  0x92, 0x76, 0x8d, 0x32, 0x45, 0xa1, 0xe7, 0xf5, 0x05, 0xd6, 0x2c, 0x67, 0x63, 0x10, 0xfa, 0xde,
  0x80, 0xc7, 0x5b, 0x96, 0x0f, 0x24, 0x50, 0x78, 0x30, 0xe5, 0x89, 0xf3, 0x73, 0xfa, 0x40, 0x11,
  0xd5, 0x26, 0xb8, 0x36, 0x96, 0x98, 0xe6, 0xbd, 0x73, 0x62, 0x56, 0xb9, 0xea, 0x28, 0x16, 0x93,
  0x5b, 0x33, 0xae, 0x83, 0xf9, 0x1f, 0xee, 0xef, 0xc8, 0xbf, 0xc7, 0xb1, 0x47, 0x43, 0xa1, 0xc6,
  0x1a, 0x64, 0x47, 0x02, 0x40, 0x3e, 0xbc, 0x0f, 0x80, 0x71, 0x5c, 0x44, 0x60, 0xbc, 0x78, 0x2e,
  0xd2, 0x77, 0xf8, 0x6e, 0x12, 0x51, 0x89, 0xdb, 0x90, 0x64, 0xcd, 0x76, 0x10, 0x29, 0x73, 0xc2,
  0x2f, 0x94, 0x7b, 0x98, 0xcd, 0xbb, 0x61, 0x16, 0x1d, 0x52, 0x11, 0x73, 0x48, 0xe6, 0x39, 0xfc,
  0xd6, 0x2d,
};

namespace bssl {

static bool Test(const uint8_t *der, size_t der_len) {
  const uint8_t *data = der;
  ScopedX509_SIG sig(d2i_X509_SIG(NULL, &data, der_len));
  if (sig.get() == NULL || data != der + der_len) {
    fprintf(stderr, "d2i_X509_SIG failed or did not consume all bytes.\n");
    return false;
  }

  static const char kPassword[] = "testing";
  ScopedPKCS8_PRIV_KEY_INFO keypair(PKCS8_decrypt(sig.get(), kPassword, -1));
  if (!keypair) {
    fprintf(stderr, "PKCS8_decrypt failed.\n");
    ERR_print_errors_fp(stderr);
    return false;
  }

  return true;
}

}  // namespace bssl

int main(int argc, char **argv) {
  if (!bssl::Test(kDER, sizeof(kDER))) {
    return 1;
  }

  printf("PASS\n");
  return 0;
}
