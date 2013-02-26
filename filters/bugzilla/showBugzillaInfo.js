//add bugzilla information to the UI
//the message processor adds a div with a specific ID to the message
//this ID is passed to the bugzilla RPC instance as an ID, which is returned in the query
//when this callback function is run, we extract that ID and update the contents accordingly

function showBugCallback(response)
{
    var id = response["id"];
    var bug = response["result"]["bugs"][0];

    var bugId = bug["id"];

    var summary = bug["summary"]
    var status = bug["status"]
    var resolution = bug["resolution"]

    var html = "[BUG "+bugId+"] "+summary + " " + status;

    if (status == "RESOLVED") {
        html += " (" + resolution +")";
    }

    document.getElementById(id).innerHTML = html;
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
