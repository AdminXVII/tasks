/* URL */

var url;
var urlNode = document.getElementById("url");
var fetchable = true;

urlNode.pattern = '((https:\\/\\/)?' +                           // secure?
        '(([a-zA-Z\\d][a-zA-Z\\d-]*[a-zA-Z\\d]\\.)+[a-z]{2,}|' +    // domain name
            '(\\d{1,3}\\.){3}\\d{1,3}))?' +               // or IPv4
'(:\\d{1,4})?' +                                  // port
'(\\/[\\w%.~+-]*)*' +                                   // filepath
'(\\?[\\w;&%.~+=-]*)?' +                                // query string
'(#[\\w-]*)?';                              // fragment locator
urlNode.value = (localStorage.getItem("url") || "chezxavier.qc.to/tasks/help.json");
setUrl(urlNode.value);
urlNode.onkeydown = function(evt){
    document.getElementById("url").className = "";
    if(evt.keyCode == 13){
        setUrl(urlNode.value);
    }
}

function setUrl(newUrl){
    var regex = /^(?:(https:\/\/)?((?:(?:[a-z\d][a-z\d-]*[a-z\d]\.)+[a-z]{2,}|(?:\d{1,3}\.){3}\d{1,3})))?(:\d{1,4})?((?:\/[\w%.~+-]*)*(?:\?[\w;&%.~+=-]*)?(?:#[\w-]*)?)$/i;
    var match = regex.exec(newUrl);
    if (match != null){
        url = (match[1]? match[1] : "http://") + (match[2]? match[2] : "localhost") + (match[3]? match[3] : "") + (match[4]? match[4] : "");
        fetchable = true;
        // Save the data in localStorage
        fetch();
    }
}

/* auto-update */

setInterval(fetch,1500);
fetch();

function fetch(){
    if(!fetchable) return;
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = update;
    xhttp.open("GET", url, true);
    xhttp.send();
}

function update(){
    if (this.readyState == 4 && this.status == 200) {
        var response = JSON.parse(this.responseText);
        resetRunning();
        response[1].forEach(function(task, idx, array){
            document.body.appendChild(newFail(task[0],task[1]));
        });
        response[0].forEach(function(task, idx, array){
            document.body.appendChild(newRun(task[0],task[1]));
        });
        document.getElementById("url").className = "";
        localStorage.setItem("url", urlNode.value);
    } else if (this.readyState == 4) {
        fetchable = false;
        resetRunning();
        document.getElementById("url").className = "notfound";
    }
}

/* Task display */

function newRun(title, msg){
    var newRun = document.createElement('div');
    newRun.className = "running";
    newRun.innerHTML = "<h1>"+title+"</h1><pre>"+msg+"</pre>";
    return newRun;
}

function newFail(title, msg){
    var newFail = document.createElement('div');
    newFail.className = "failed";
    newFail.innerHTML = "<h1><button onclick='removeElement(this.parentNode.parentNode);'>&times;</button>"+title+"</h1><pre>"+msg+"</pre>";
    return newFail;
}

/* Helpers */

function resetRunning(){
    var elements = document.getElementsByClassName("running");
    while(elements.length > 0){
        removeElement(elements[0]);
    }
}

function removeElement(el){
    el.parentNode.removeChild(el);
}
