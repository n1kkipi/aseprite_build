#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "houdini.h"

#if !defined(__has_builtin)
# define __has_builtin(b) 0
#endif

#if !__has_builtin(__builtin_expect)
# define __builtin_expect(e, v) (e)
#endif

#define likely(e) __builtin_expect((e), 1)
#define unlikely(e) __builtin_expect((e), 0)

/**
 * According to the OWASP rules:
 *
 * & --> &amp;
 * < --> &lt;
 * > --> &gt;
 * " --> &quot;
 * ' --> &#x27;     &apos; is not recommended
 * / --> &#x2F;     forward slash is included as it helps end an HTML entity
 *
 */
static const char HTML_ESCAPE_TABLE[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 2, 3, 0, 0, 0, 0, 0, 0, 0, 4,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static const char *HTML_ESCAPES[] = {"",      "&quot;", "&amp;", "&#39;",
                                     "&#47;", "&lt;",   "&gt;"};

int houdini_escape_html(cmark_strbuf *ob, const uint8_t *src, bufsize_t size,
                         int secure) {
  bufsize_t i = 0, org, esc = 0;

  while (i < size) {
    org = i;
    while (i < size && (esc = HTML_ESCAPE_TABLE[src[i]]) == 0)
      i++;

    if (i > org)
      cmark_strbuf_put(ob, src + org, i - org);

    /* escaping */
    if (unlikely(i >= size))
      break;

    /* The forward slash is only escaped in secure mode */
    if ((src[i] == '/' || src[i] == '\'') && !secure) {
      cmark_strbuf_putc(ob, src[i]);
    } else {
      cmark_strbuf_puts(ob, HTML_ESCAPES[esc]);
    }

    i++;
  }

  return 1;
}
