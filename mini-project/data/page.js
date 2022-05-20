
class Timer {
  constructor(root)
  {
    this.el = {
      control: document.getElementById("start"),
      reset: document.getElementById("reset"),
      response: document.getElementById("response")
    };

    this.el.control.addEventListener("click", () => {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/start", true);
      xhr.send();
    });

    this.el.reset.addEventListener("click", () => {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/reset", true);
      xhr.send();

    });
    this.el.response.addEventListener("click", () => {
      this.response_order();
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/response", true);
      xhr.send();
    });
    this.getData();
  }

  response_order(){
    console.log("response order");
  }
  getData() {
    console.log("getData")
    setInterval(function ( ) {
      console.log(".");
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("orderStatus").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "/order", true);
      xhttp.send();
    }, 1000 ) ;

    setInterval(function ( ) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("time").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "/time", true);
      xhttp.send();
    }, 1000 ) ;

  }
}

new Timer();
