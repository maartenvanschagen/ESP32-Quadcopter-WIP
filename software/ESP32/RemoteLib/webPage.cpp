#include <WString.h>
#include "webPage.h"

namespace webPage {
  const String page = R"(

<html>
    <head>
        <title>drone</title>
        
        <script> // websockets
            var websock;
            var pitch = 0;
            var roll = 0;
            var trottle = 0;
            var yaw = 0;
            function start() {
                setSensitivity();
                websock = new WebSocket('ws://' + window.location.hostname + '/ws');
                websock.onopen = function(evt) { console.log('websock open'); };
                websock.onclose = function(evt) { console.log('websock close'); };
                websock.onerror = function(evt) { console.log(evt); };
                websock.onmessage = function(evt) { console.log(evt);};
            }
        </script>
        
        <script> // joystick
            
            var joystick = function(parent){
                var self = this;
                this.parent = parent;
                this.status = null;
                this.jumpbackx = false;
                this.jumpbacky = false;
                this.below0x = true;
                this.below0y = true
                this.xSensitivity = 100;
                this.ySensitivity = 100;
                
                this.getStatus = function(){ //gets status element
                    for(i = 0; i < this.parent.childNodes.length; i++){
                        item = this.parent.childNodes[i];
                        if(item.id == "statusDiv"){
                            return item;
                        }
                    }
                }
                
                this.startx = null; //startposition of finger
                this.starty = null;
                this.lasttotalx = 0;
                this.lasttotaly = 0;
                this.lastmoved = 'x';
                this.moved = false; //true if movement was more then the rejection value
                this.movedbefore = false;
                
                this.touchmove = function(touchobj, functionName){
                    x = parseInt(touchobj.clientX); //position of finger
                    y = parseInt(touchobj.clientY);
                    
                    distxraw = x - this.startx;//distance of startposition of finger in pixels bevore rejection
                    distyraw = this.starty - y;
                    
                    //sets moved to true if finger has been moved more than the rejection value
                    if((-distxraw>0 ? -distxraw : distxraw) > rejection || (-distyraw>0 ? -distyraw : distyraw) > rejection){
                        this.moved = true;
                    }
                    
                    distxpx = this.moved ? distxraw : 0; //distance of startposition of finger in pixels after rejection
                    distypx = this.moved ? distyraw : 0;
                    
                    distx = distxpx * (this.xSensitivity / 100); //distance of startposition of finger times the sensitivity
                    disty = distypx * (this.ySensitivity / 100);
                    
                    totalx = this.lasttotalx + distx; //total movement of x after letting go
                    totaly = this.lasttotaly + disty;
                    
                    //sets totalx or y back to zero when below it and it is not allowed to
                    if(this.below0x === false && totalx < 0){
                        totalx = 0;
                    }
                    
                    if(this.below0y === false && totaly < 0){
                        totaly = 0;
                    }
                    
                    //only do if the function is called by touchend
                    if(functionName == "touchend"){
                        //if the movement was more then the rejection value, lastmoved wil be x if x has been moved more, or y if y has been moved more
                        if(this.moved && (-distxpx>0 ? -distxpx : distxpx) > (-distypx>0 ? -distypx : distypx)){
                            this.lastmoved = 'x';
                        }else if(this.moved && (-distypx>0 ? -distypx : distypx) > (-distxpx>0 ? -distxpx : distxpx)){
                            this.lastmoved = 'y';
                        }
                        
                        //reset totalx or totaly if a click was detected
                        if(!this.moved){
                            if(this.jumpbackx && this.jumpbacky){      //cancel if both axies jump back
                            }else if(this.jumpbackx && !this.jumpbacky){
                                totaly = 0;
                            }else if(!this.jumpbackx && this.jumpbacky){
                                totalx = 0;
                            }else if(!this.movedbefore){
                                totalx = 0;
                                totaly = 0;
                            }else if(this.lastmoved == 'x'){
                                totalx = 0;
                            }else if(this.lastmoved == 'y'){
                                totaly = 0;
                            }
                        }
                        
                        //handle jumpbacks
                        if (this.jumpbackx) totalx = 0;
                        if (this.jumpbacky) totaly = 0;
                        
                        this.movedbefore = this.moved;
                        this.lasttotalx = totalx;
                        this.lasttotaly = totaly;
                    }
                    
                    this.sendX(totalx);
                    this.sendY(totaly);
                    
                    this.status.innerHTML = 'functionName: ' + functionName + '<br>lastmoved: ' + this.lastmoved + '<br>moved: ' + this.moved + '<br> x: ' + x + 'px<br>y: ' + y + 'px<br>distx: ' + distx + '<br>disty: ' + disty + '<br>totalx: ' + totalx + '<br>totaly: ' + totaly;
                    
                }
                
                this.onstart = function(e){
                    this.moved = false; //reset moved
                    touchobj = e.targetTouches[0];
                    this.startx = parseInt(touchobj.clientX); //position of finger
                    this.starty = parseInt(touchobj.clientY);
                    this.status.innerHTML = 'functionName: touchstart<br>lastmoved: ' + this.lastmoved + '<br>moved: ' + this.moved + '<br> x: ' + this.startx + 'px<br>y: ' + this.starty + 'px';
                    e.preventDefault();
                }
                
                this.onmove = function(e){
                    this.touchmove(e.targetTouches[0], "touchmove");
                    e.preventDefault();
                }
                
                this.onend = function(e){
                    this.touchmove(e.changedTouches[0], "touchend");
                    e.preventDefault();
                }
                
                
                parent.addEventListener('touchstart', function(e){
                    self.onstart(e);
                });
                
                parent.addEventListener('touchmove', function(e){
                    self.onmove(e);
                });
                
                parent.addEventListener('touchend', function(e){
                    self.onend(e);
                });
                
                parent.addEventListener('touchcancel', function(e){
                    self.onend(e);
                });
                
                this.sendX = function(totalx){ console.log("X: " + totalx) }
                this.sendY = function(totaly){ console.log("Y: " + totaly) }
                
                return this;
            }
            </script>
    </head>
    <body style="overflow:hidden;" onload="javascript:start();">
        <div class="controlBar" style="border-style:solid; border-width:1; width:99%; height:14%; overflow:hidden; position:absolute; left:0px; top:0px;">
            <script>
                var sensitivity = 10;
                var thrustSensitivity = 40;
                function updateSensitivity(value){
                    value = parseInt(value);
                    sensitivity = value;
                    document.getElementById("sensitivitySlider").value = value;
                    document.getElementById("sensitivityForm").value = value;
                    joystickL.xSensitivity = sensitivity;
                    joystickR.xSensitivity = sensitivity;
                    joystickR.ySensitivity = sensitivity;
                }
                function updateThrustSensitivity(value){
                    value = parseInt(value);
                    thrustSensitivity = value;
                    document.getElementById("thrustSensitivitySlider").value = value;
                    document.getElementById("thrustSensitivityForm").value = value;
                    joystickL.ySensitivity = thrustSensitivity;
                }
                
                function setSensitivity(){
                    //set the initial sensitivity values at page load
                    updateSensitivity(sensitivity);
                    updateThrustSensitivity(thrustSensitivity);
                }
            </script>
            <label for="sensitivitySlider">Sensitivity</label>
            <input type="range" min="0" max="200" value="100" id="sensitivitySlider" step="1" oninput="updateSensitivity(value);">
            <input type="number" id="sensitivityForm" value="100" oninput="updateSensitivity(value);" style="width:50px;">
            <label for="thrustSensitivitySlider">Thrust Sensitivity</label>
            <input type="range" min="0" max="200" valTe="100" id="thrustSensitivitySlider" step="1" oninput="updateThrustSensitivity(value);">
            <input type="number" id="thrustSensitivityForm" value="100" oninput="updateThrustSensitivity(value);" style="width:50px;">
            
            <script>
                var rejection = 1; //how many pixels movement it needs before program sees it as movement
                function updateRejection(value){
                    value = parseInt(value);
                    rejection = value;
                    document.getElementById("rejectionSlider").value = value;
                    document.getElementById("rejectionForm").value = value;
                }
            </script>
            <label for="rejectionSlider">Rejection</label>
            <input type="range" min="1" max="30" value="1" id="rejectionSlider" step="1" oninput="updateRejection(value);">
            <input type="number" id="rejectionForm" value="1" oninput="updateRejection(value);" style="width:50px;">
            
            <p id="connectState"> Loading... </p>
            <script>
                setInterval(reloadConnectState,500);
                var loadingDots = 0;
                var addDots = false;
                function reloadConnectState(){
                    loadingDots = (loadingDots)%3 + 1;
                    if(websock !== undefined && websock.readyState === WebSocket.OPEN){
                        document.getElementById("connectState").innerHTML = "Connected";
                        addDots = false;
                    }else if(websock === undefined){
                         document.getElementById("connectState").innerHTML = "Loading";
                         addDots = true;
                    }else if(websock.readyState === WebSocket.CONNECTING){
                        document.getElementById("connectState").innerHTML = "Connecting";
                        addDots = true;
                    }else{
                        document.getElementById("connectState").innerHTML = "Disconnected";
                        addDots = false;
                    }
                    if(addDots === true){
                        if(loadingDots === 1){
                            document.getElementById("connectState").innerHTML += ".";
                        }else if(loadingDots === 2){
                            document.getElementById("connectState").innerHTML += "..";
                        }else if(loadingDots === 3){
                            document.getElementById("connectState").innerHTML += "...";
                        }
                    }
                }
            </script>
            
        </div>
        
        <div class="touchbox" id="boxL" style="border-style:solid; border-width:1; width:49%; height:84%; overflow:hidden; position:absolute; left:0px; top:15%;">
            <h3 id="statusDiv">Status</h3>
            <script>
                var joystickL = new joystick(document.currentScript.parentNode);
                joystickL.status = joystickL.getStatus();
                joystickL.jumpbackx = true;
                joystickL.jumpbacky = false;
                joystickL.below0y = false;
                joystickL.xSensitivity = sensitivity;
                joystickL.ySensitivity = thrustSensitivity;
                
                joystickL.sendX = function(x){
                    if(websock.readyState === WebSocket.OPEN){
                        websock.send("y" + String(x));
                        console.log("yaw:" + String(x));
                    }
                }
                
                joystickL.sendY = function(y){
                    if(websock.readyState === WebSocket.OPEN){
                        websock.send("t" + String(y));
                        console.log("trottle:" + String(y));
                    }
                }
            </script>
        </div>
        
        <div class="touchbox" id="boxR" style="border-style:solid; border-width:1; width:49%; height:84%; overflow:hidden; position:absolute; right:0px; top:15%;">
            <h3 id="statusDiv">Status</h3>
            <script>
                var joystickR = new joystick(document.currentScript.parentNode);
                joystickR.status = joystickR.getStatus();
                joystickR.jumpbackx = true;
                joystickR.jumpbacky = true;
                joystickR.xSensitivity = sensitivity;
                joystickR.ySensitivity = sensitivity;
                
                joystickR.sendX = function(x){
                    if(websock.readyState === WebSocket.OPEN){
                        websock.send("r" + String(x));
                        console.log("roll:" + String(x));
                    }
                }
                
                joystickR.sendY = function(y){
                    if(websock.readyState === WebSocket.OPEN){
                        websock.send("p" + String(y));
                        console.log("pitch:" + String(y));
                    }
                }
            </script>
        </div>
    </body>
</html>
  )";
}
