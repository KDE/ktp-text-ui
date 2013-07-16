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

function showEmbedlyCallback(response)
{
    var title = response["title"];

    if (response["type"] == "error") {
        return;
    }

    switch (response["type"]) {
        case "error":
            return;
        case "link":
            break;
        case "video":
        case "rich":
            break;
        case "photo":
            break;
        default:
            // WTF?
            return;
    }

    document.getElementById(id).innerHTML = escapeHTML(text);
}

//use jsonp to avoid problems with web security origin
//see http://en.wikipedia.org/wiki/JSONP

//this function creates a new <script src = "http://bugs.kde.org/jsonrpc.cgi?=
function embedly_processUrl(jsonCallUrl)
{
    var script = document.createElement('script');
    script.src = jsonCallUrl
    document.getElementsByTagName('head')[0].appendChild(script);
}
