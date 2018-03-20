var express = require('express'),
  app = express(),
  sse = require('./sse');

app.use(sse);

/* Routes setup */

app.get('/', function(req, res) {
  res.sseSetup();
  connections.push(res);
  
  let interval = setInterval(function() {
    let uid = 0;
    if (uids.length === 0){
      add(12,50);
      return;
    }
    let num = Math.random();
      /* New task */
    if (num >= 0.999){ /* End task */
      end();
    } else if (num < 0.999/(uids.length*uids.length*uids.length)){
      add(12, 50);
    } else { /* New random message */
      msg(1000);
    }
  }, 1);
  setTimeout(function(){clearInterval(interval);console.log("\n\nNew sequence\n\n");connections = []; uids = [];}, 5000);
});

function add(uid_len, name_len){
  uid = random(uid_len);
  uids.push(uid);
  broadcast("new", { [uid]: random(name_len) });
  console.log("new", uid);
}

function end(){
  uid = uids[uids.length * Math.random() << 0].toString();
  broadcast("end", { [uid]: "" });
  for(var i = uids.length - 1; i >= 0; i--) {
    if(uids[i] === uid) {
      uids.splice(i, 1);
    }
  }
  console.log("end", uid);
}

function msg(len){
  uid = uids[Math.floor(uids.length * Math.random())].toString();
  broadcast("msg", { [uid]: random(len) });
  console.log("msg", uid);
}

app.listen(9000, function() {
  console.log('Listening on port 9000...');
});

/* Main logic */

var connections = [],
  uids = [];
  
function broadcast(event, data){
  for(var i = 0; i < connections.length; i++) {
    connections[i].sseSend(event, data);
  }
}

function random(len){
  var text = "";
  var possible = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

  for (var i = 0; i < len; i++)
    text += possible.charAt(Math.floor(Math.random() * possible.length));

  return text;
}