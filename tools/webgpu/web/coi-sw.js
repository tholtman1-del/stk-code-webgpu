// Service worker that makes the page cross-origin isolated on static hosts
// that cannot send the COOP/COEP headers themselves (e.g. GitHub Pages).
// index.html registers it only when the page is not isolated already, and
// reloads once it controls the page. Every response gets the headers that
// serve.py (or the host's _headers file) would send.
self.addEventListener('install', () => self.skipWaiting());
self.addEventListener('activate', (event) => event.waitUntil(self.clients.claim()));

self.addEventListener('fetch', (event) => {
  const request = event.request;
  // Chrome throws for these when they are not same-origin
  if (request.cache === 'only-if-cached' && request.mode !== 'same-origin') return;
  event.respondWith(fetch(request).then((response) => {
    // Opaque responses cannot be changed
    if (response.status === 0) return response;
    const headers = new Headers(response.headers);
    headers.set('Cross-Origin-Opener-Policy', 'same-origin');
    headers.set('Cross-Origin-Embedder-Policy', 'require-corp');
    headers.set('Cross-Origin-Resource-Policy', 'same-origin');
    return new Response(response.body, {
      status: response.status,
      statusText: response.statusText,
      headers,
    });
  }));
});
