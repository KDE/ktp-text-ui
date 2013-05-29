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

var expandUrlCallbacks={};

function expandUrlCallback(response,id)
{
    var text = response["long-url"];
    document.getElementById(id).innerHTML += "<a href ='" + escapeHTML(text) +"'>"+ escapeHTML(text) +"</a><br />";
}

function showShortUrl(jsonCallUrl, id)
{
    expandUrlCallbacks[id] = function(response) { //create a new callback function
        delete expandUrlCallbacks[id]
        expandUrlCallback(response,id);
        };

    var script = document.createElement('script');
    script.src = jsonCallUrl;
    document.getElementsByTagName('head')[0].appendChild(script);
}
