/*
 * Main program for the UDP executable spec
 *
 * Copyright 2017 Patrick Gauvin
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "config.h"
#include "rx.h"
#include "tx.h"

void
usage (char *name)
{
  fprintf (stderr,
           "Usage:\n"
           "\t%s <rx|tx> <port> <len>\n"
           "\nInput is read from stdin, output is sent to stdout.\n"
           "\nRX Mode:\n"
           "Input is consecutive raw IP datagrams of length len. Output is\n"
           "raw UDP payloads if their datagram is valid.\n"
           "\nTX Mode:\n"
           "Input is consecutive raw UDP data payloads of length len. Output\n"
           "is raw UDP datagrams with the input data as their payloads.\n",
           name);
}

int
main (int argc, char **argv)
{
  int status;
  FILE *fp_in, *fp_out;
  uint8_t buf_in[IP_MAX_DGRAM_LEN], buf_out[IP_MAX_DGRAM_LEN];
  size_t n;
  bool rx;
  size_t dgram_len;
  uint16_t buf_out_len;
  uint32_t addr_src;
  uint16_t port, port_src;

  if (argc < 4)
    {
      fprintf (stderr, "Not enough arguments\n");
      usage (argv[0]);
      return EXIT_FAILURE;
    }
  if (0 == strcmp (argv[1], "rx"))
    rx = true;
  else if (0 == strcmp (argv[1], "tx"))
    rx = false;
  else
    {
      fprintf (stderr, "Invalid argument\n");
      usage (argv[0]);
      return EXIT_FAILURE;
    }
  port = atoi (argv[2]);
  dgram_len = atoi (argv[3]);

  /* TODO */
  if (!rx)
    {
      fprintf (stderr, "TX mode not supported yet\n");
      return EXIT_FAILURE;
    }

  fp_in = stdin;
  fp_out = stdout;
  while (0 == feof (fp_in))
    {
      n = fread (buf_in, dgram_len, 1, fp_in);
      if (1 != n)
        {
          if (feof (fp_in))
            break;
          else
            {
              fprintf (stderr, "Input file read error\n");
              status = EXIT_FAILURE;
              goto err;
            }
        }
      if (rx)
        status = udp_rx (true, port, buf_in, dgram_len, buf_out, &buf_out_len,
                         &port_src, &addr_src);
      else
        status = udp_tx (true, port, 0, buf_in, dgram_len, buf_out,
                         &buf_out_len);
      if (0 != status)
        {
          fprintf (stderr, "Transfer error: %x\n", status);
          status = EXIT_FAILURE;
          goto err;
        }
      else
        n = fwrite (buf_out, buf_out_len, 1, fp_out);
      assert (1 == n);
    }
  status = EXIT_SUCCESS;

err:
  assert (0 == fclose (fp_out));
  assert (0 == fclose (fp_in));

  return status;
}