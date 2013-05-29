//escape HTML special symbols
//this function escapes symbols &<>"' with entities
//the chars-to-codes map is created only once, not on every call

var escapeHTML = (function() {
    'use strict';
    var codes = {
        '&': '&amp;',
        '<': '&lt;',
        '>': '&gt;',
        '"': '&quot;',
        "'": '&#39;'
    };
    return function(string) {
        return string.replace(/[&<>"']/g, function(char) {
            return codes[char];
        });
    };
}());

//add bugzilla information to the UI
//the message processor adds a div with a specific ID to the message
//this ID is passed to the bugzilla RPC instance as an ID, which is returned in the query
//when this callback function is run, we extract that ID and update the contents accordingly

function showBugCallback(response)
{
    var id = response["id"];

    if (!response["result"])
            return;

    var bug = response["result"]["bugs"][0];

    var bugId = bug["id"];

    var summary = bug["summary"]
    var status = bug["status"]
    var resolution = bug["resolution"]

    var text = "[BUG " + bugId + "] " + summary + " " + status;

    if (status == "RESOLVED") {
        text += " (" + resolution +")";
    }

    document.getElementById(id).innerHTML = escapeHTML(text);
}

//use jsonp to avoid problems with web security origin
//see http://en.wikipedia.org/wiki/JSONP

//this function creates a new <script src = "http://bugs.kde.org/jsonrpc.cgi?=
function showBug(jsonCallUrl)
{
    var script = document.createElement('script');
    script.src = jsonCallUrl
    document.getElementsByTagName('head')[0].appendChild(script);
}
