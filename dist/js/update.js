/* Register service worker */
if ('serviceWorker' in navigator) {
  window.addEventListener('load', function() {
    navigator.serviceWorker.register('sw.js');
  });
}

/* URL source */

var url;
var urlNode = document.getElementById("url");
var source;

urlNode.value = (localStorage.getItem("url") || "help");
setUrl(urlNode.value);
urlNode.onkeydown = function(evt){
  if(evt.keyCode == 13){
    urlNode.blur();
    setUrl(urlNode.value);
  }
};

function setUrl(newUrl){
    var regex = /^(?:((?:(?:[a-z\d][a-z\d-]*[a-z\d]\.)+[a-z]{2,}|(?:\d{1,3}\.){3}\d{1,3})))?(:\d{1,4})?((?:\/[\w%.~+-]*)*(?:\?[\w;&%.~+=-]*)?(?:#[\w-]*)?)$/i;
    var match = newUrl.match(regex);
    if (match !== null){
        urlNode.setCustomValidity("");
        url = "http://" + (match[1]? match[1] : "localhost") + (match[2]? match[2] : ":9000") + (match[3]? match[3] : "");
        if (source) source.close();
        source = new EventSource(url);
        
        source.addEventListener("new", eventNew, false);
        source.addEventListener("msg", eventMsg, false);
        source.addEventListener("end", eventEnd, false);
        source.onerror = onError;
        source.onopen = opened;
        localStorage.setItem("url", urlNode.value);
    } else if (newUrl == "help"){
        // show help
    }
}

/* Helpers */
function removeElement(el){
    el.parentNode.removeChild(el);
}

function eventNew(e){
  var resp = JSON.parse(e.data);
  for (let id in resp)
    updates[id] = add(id, resp[id]);
}

function eventMsg(e){
  var resp = JSON.parse(e.data);
  for (let id in resp)
    update(updates[id], resp[id]);
}

function eventEnd(e){
  var resp = JSON.parse(e.data);
  for (let id in resp){
    end(updates[id]);
    delete updates[id];
  }
}

function onError(e) {
  alert("There was a problem with communication");
  console.log(e);
  source.close();
}

function opened(e){
}

/* Debugging */

function random(len){
  var text = "";
  var possible = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

  for (var i = 0; i < len; i++)
    text += possible.charAt(Math.floor(Math.random() * possible.length));

  return text;
}

/* Main logic*/

const main = document.getElementById('updates');
const template = document.getElementById('update-template').content;
const closeTemplate = document.getElementById('close-template').content;

updates = {};

/* Direct node manipulation*/

function add(id, name){
  var node = document.importNode(template, true);
  node.querySelector(".title").textContent = name;
  out = {
    text:  node.querySelector(".content"),
    title: node.querySelector(".title")
  };
  main.appendChild(node);
  
  out.node = out.title.parentNode.parentNode;
  return out;
}

function rename(node, name) {
  node.title.textContent = name;
}

function update(node, text) {
  node.text.textContent = text;
}

function end(node) {
  var closeButton = document.importNode(closeTemplate, true);
  closeButton.querySelector("button").onclick = function(){removeElement(node.node);};
  node.node.appendChild(closeButton);
  node.node.className += " ended";
  main.insertBefore(node.node, main.firstChild);
}