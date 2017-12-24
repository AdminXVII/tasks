/* URL */

var url;
var urlNode = document.getElementById("url");
var source;

urlNode.value = (localStorage.getItem("url") || "help");
setUrl(urlNode.value);
urlNode.onkeydown = function(evt){
    document.getElementById("url").className = "";
    if(evt.keyCode == 13){
        setUrl(urlNode.value);
    }
};

function setUrl(newUrl){
    var regex = /^(?:(https:\/\/)?((?:(?:[a-z\d][a-z\d-]*[a-z\d]\.)+[a-z]{2,}|(?:\d{1,3}\.){3}\d{1,3})))?(:\d{1,4})?((?:\/[\w%.~+-]*)*(?:\?[\w;&%.~+=-]*)?(?:#[\w-]*)?)$/i;
    var match = newUrl.match(regex);
    if (match !== null){
        urlNode.setCustomValidity("");
        url = (match[1]? match[1] : "http://") + (match[2]? match[2] : "localhost") + (match[3]? match[3] : ":9000") + (match[4]? match[4] : "");
        if (source) source.close();
        source = new EventSource(url);
        
        source.onmessage = message;
        source.onerror = function(e) {
          alert("EventSource failed.");
        };
        localStorage.setItem("url", urlNode.value);
    } else if (newUrl == "help"){
        // show help
    } else {
        urlNode.setCustomValidity("Invalid URL.");
    }
}

/* auto-update */


function message(e) {
    var response = JSON.parse(e.data);
    var id = response[0],
        type = response[1];
    
    if(type === 0){
        rename(id, response[2]);
    } else if (type == 1){
        update(id, response[2]);
    } else {
        end(id);
    }
}


function rename(id, title){
    var el = document.getElementById(id);
    if (el === null){
        newRun(id,title);
    } else {
        el.getElementsByTagName('h1')[0].innerHTML = title;
    }
}

function update(id, message){
    el = document.getElementById(id).getElementsByTagName('pre')[0].innerHTML = message;
}

function end(id){
    el = document.getElementById(id);
    if (el === null)
        return;
    el.className = "failed";
    var button = document.createElement("button");
    button.onclick = function(){removeElement(this.parentNode.parentNode)};
    button.innerHTML = "&times";
    el.getElementsByTagName('h1')[0].prepend(button);
}

/* Task display */
function newRun(id, title){
    var newRun = document.createElement('div');
    newRun.id = id;
    newRun.className = "running";
    newRun.innerHTML = "<h1>"+title+"</h1><pre></pre>";
    document.body.appendChild(newRun);
}

/* Helpers */
function removeElement(el){
    el.parentNode.removeChild(el);
}
