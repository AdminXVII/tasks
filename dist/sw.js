importScripts('workbox-sw.prod.v2.1.3.js');

/**
 * DO NOT EDIT THE FILE MANIFEST ENTRY
 *
 * The method precache() does the following:
 * 1. Cache URLs in the manifest to a local cache.
 * 2. When a network request is made for any of these URLs the response
 *    will ALWAYS comes from the cache, NEVER the network.
 * 3. When the service worker changes ONLY assets with a revision change are
 *    updated, old cache entries are left as is.
 *
 * By changing the file manifest manually, your users may end up not receiving
 * new versions of files because the revision hasn't changed.
 *
 * Please use workbox-build or some other tool / approach to generate the file
 * manifest which accounts for changes to local files and update the revision
 * accordingly.
 */
const fileManifest = [
  {
    "url": "apple-touch-icon.png",
    "revision": "62196098d0a67ca245e238a79c624528"
  },
  {
    "url": "css/style.css",
    "revision": "e9d7c1d28ca38f7968f4c6e240945731"
  },
  {
    "url": "img/err404.svg",
    "revision": "59b7778265da7a145e1752e1d07844d6"
  },
  {
    "url": "img/logo.svg",
    "revision": "285b6b775bbe042ddb5236672f077702"
  },
  {
    "url": "index.html",
    "revision": "fcc1e24d67761f2e62128237bd1518da"
  },
  {
    "url": "js/update.js",
    "revision": "eb8644e73d6c270c47b9147304a2e366"
  },
  {
    "url": "logo-16x16.png",
    "revision": "d41d8cd98f00b204e9800998ecf8427e"
  },
  {
    "url": "logo-192x192.png",
    "revision": "b476fc2764930f7f9e46ccae0cd742c3"
  },
  {
    "url": "logo-32x32.png",
    "revision": "222a688bd5a0fbfbb80c266f7889f7d7"
  },
  {
    "url": "logo-48x48.png",
    "revision": "7397c0dbc2644385aea973ffeb48124a"
  },
  {
    "url": "logo-512x512.png",
    "revision": "b21bb9f73daa6235dd078fa3c4dfb019"
  },
  {
    "url": "logo-96x96.png",
    "revision": "f5a9e55c997f7e99baeea124cb7e8123"
  },
  {
    "url": "logo.svg",
    "revision": "5edc7f349232fee91cb8559e3b6e5ced"
  },
  {
    "url": "manifest.json",
    "revision": "e530efe9b8ab1dc6bff5f04d343ba3bd"
  },
  {
    "url": "update.js",
    "revision": "3ba734b6da10c634e36c2d5b292c2904"
  }
];

const workboxSW = new self.WorkboxSW({
  "skipWaiting": true,
  "clientsClaim": true
});
workboxSW.precache(fileManifest);
workboxSW.router.registerRoute(/^https:\/\/code\.getmdl\.io/, workboxSW.strategies.staleWhileRevalidate({}), 'GET');
workboxSW.router.registerRoute(/^https:\/\/fonts\.googleapis\.com/, workboxSW.strategies.staleWhileRevalidate({}), 'GET');
workboxSW.router.registerRoute(/^https:\/\/fonts\.gstatic\.com/, workboxSW.strategies.staleWhileRevalidate({}), 'GET');
