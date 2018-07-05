var express = require('express'),
  app = express(),
  sse = require('./sse');

app.use(sse);

/* Routes setup */


app.get('/', function(req, res) {
  console.log("\n\nNew sequence\n\n");
  res.sseSetup();
  var events = [];
  
  let interval = setInterval(randomEvent, 1, events, res);
  setTimeout(function(){clearInterval(interval);res.sseClose();}, 5000);
});

function randomEvent(uids, res){
  let uid;
  let num = Math.random();
    /* New task */
  if (num < 0.999/(uids.length*uids.length*uids.length) || uids.length === 0){
    uid = random(50);
    res.sseSend("new", { [uid]: random(20) });
    uids.push(uid);
  } else if (num >= 0.999){ /* End task */
    uid = uids[Math.floor(uids.length * Math.random())].toString();
    res.sseSend("end", { [uid]: "" });
    for(var i = uids.length - 1; i >= 0; i--) {
      if(uids[i] === uid) {
        uids.splice(i, 1);
      }
    }
  } else { /* New random message */
    uid = uids[Math.floor(uids.length * Math.random())].toString();
    res.sseSend("msg", { [uid]: random(1000) });
  }
}

app.listen(9000, function() {
  console.log('Listening on port 9000...');
});

/* Helper */

function random(len){
  var text = "";
  var possible = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

  for (var i = 0; i < len; i++)
    text += possible.charAt(Math.floor(Math.random() * possible.length));

  return text;
}