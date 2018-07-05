/* Register service worker */
if ('serviceWorker' in navigator) {
  window.addEventListener('load', function() {
    navigator.serviceWorker.register('sw.js');
  });
}

/* URL source */

var url;
const urlNode = document.getElementById("url");
const snackbar = document.getElementById('snackbar');
var source;

window.onload = function(){
  urlNode.value = localStorage.getItem("url");
  setUrl(urlNode.value);
  urlNode.parentNode.MaterialTextfield.checkDirty();
  urlNode.onkeydown = function(evt){
    if(evt.keyCode == 13){
      urlNode.blur();
      setUrl(urlNode.value);
    }
  };
};

function setUrl(newUrl){
  let url = toURL(newUrl);
  if (url === false){
    return;
  }
  connect(url);
  localStorage.setItem("url", newUrl);
}

function toURL(url){
  // https://regex101.com/r/IjsjtQ/1
  const regex = /^(?:((?:(?:[a-z\d][a-z\d-]*[a-z\d]\.)+[a-z]{2,}|(?:\d{1,3}\.){3}\d{1,3})))?(:\d{1,4})?((?:\/[\w%.~+-]*)*(?:\?[\w;&%.~+=-]*)?(?:#[\w-]*)?)$/i;
  var match = url.match(regex);
  if (match !== null){
    return (match[1]? "https://" + match[1] : "http://localhost") + (match[2]? match[2] : ":9000") + (match[3]? match[3] : "");
  }
  return false;
}

function connect(url){
  if (source){
    source.close();
    deleteAll();
    updates = {};
  }
  source = new EventSource(url);
  
  source.addEventListener("new", eventNew, false);
  source.addEventListener("msg", eventMsg, false);
  source.addEventListener("end", eventEnd, false);
  source.onerror = onError;
  source.onopen = opened;
}

/* Data binding */
updates = {};

function removeElement(el){
    el.parentNode.removeChild(el);
}

function eventNew(e){
  var resp = unpack(e);
  for (let id in resp)
    updates[id] = add(id, resp[id]);
}

function eventMsg(e){
  var resp = unpack(e);
  for (let id in resp)
    update(updates[id], resp[id]);
}

function eventEnd(e){
  var resp = unpack(e);
  for (let id in resp){
    end(updates[id]);
    delete updates[id];
  }
}

function unpack(e){
  try {
    return JSON.parse(e.data);
  }
  catch(err){
    console.log("Error unpacking JSON: ", e.data, err);
  }
}

function onError(e) {
  snackbar.MaterialSnackbar.showSnackbar({
    message: 'Error: failed to connect to remote host.',
    timeout: 2000
  });
  console.log(e);
  source.close();
}

function opened(e){
}

/* Main logic*/

var main = document.getElementById('updates');
const template = document.getElementById('update-template').content;
const closeTemplate = document.getElementById('close-template').content;

/* Direct node manipulation*/

function deleteAll(){
  let oldMain = main;
  main = oldMain.cloneNode(false);
  oldMain.parentNode.replaceChild(main, oldMain);
  console.log(main.parentNode);
}

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
  if (node === undefined) return;
  
  node.title.textContent = name;
}

function update(node, text) {
  if (node === undefined) return;
  
  node.text.textContent = text;
}

function end(node) {
  if (node === undefined) return;
  
  var closeButton = document.importNode(closeTemplate, true);
  closeButton.querySelector("button").onclick = function(){removeElement(node.node);};
  node.node.appendChild(closeButton);
  node.node.className += " ended";
  main.insertBefore(node.node, main.firstChild);
}