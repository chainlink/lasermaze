document.addEventListener('DOMContentLoaded', loadScript, false);

function loadScript() {
  console.log('script loaded');

  spark.login({accessToken: '5082fc338da2d4b6bc386f1e1c47374a7264b121'}, function() {
    console.log('logged in!');
    spark.getDevice('buttz', function(err, device) {
      if(!err) {
        //Declare Event Handlers
        var startBtn = document.getElementById('startBtn');
        var stopBtn = document.getElementById('stopBtn');
        var resetBtn = document.getElementById('resetBtn');
        console.log('device name: ', device.name);
        
        device.subscribe('Time', function(data) {
          console.log("Time: " + data);
        });

        startBtn.onclick = function() {
          console.log('starting!');
          device.callFunction('timeControl', 'start', function(err, data) {
            console.log("Start Called!");
          });
        }
        stopBtn.onclick = function() {
          console.log('stopping!');
          device.callFunction('timeControl', 'stop', function(err, data) {
            console.log("Stop Called!");
          });
        }
        resetBtn.onclick = function() {
          console.log('resettting!');

          device.callFunction('timeControl', 'reset', function(err, data) {
            console.log('Reset Called!');
          });
        }
      }
    });
  });
}
