importScripts('workbox-sw.prod.v1.0.1.js');

const workboxSW = new self.WorkboxSW();
/* workbox-cli will inject precaching in the array below*/
workboxSW.precache([]);