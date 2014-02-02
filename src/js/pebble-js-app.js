Pebble.addEventListener("ready",
  function(e) {
    console.log("JavaScript app ready and running!");
  }
);


Pebble.addEventListener("showConfiguration",
  function(e) {
    console.log("JavaScript app showing configuration.");
    Pebble.openURL('https://rawgithub.com/samalander/big-h/master/webconfig/big-h.config.html');
  }
);


Pebble.addEventListener("webviewclosed",
  function(e) {
  	var settings = JSON.parse(e.response);
    console.log("Configuration window returned: " + JSON.stringify(settings));
    if (settings.saved) {
    	delete settings.saved;
    	Pebble.sendAppMessage(settings);
    }
  }
);