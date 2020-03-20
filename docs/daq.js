(function() {
  'use strict';

  document.addEventListener('DOMContentLoaded', event => {
    let lorawanForm = document.querySelector('#lorawan-form');
    let generalForm = document.querySelector('#general-form');     
    let channelsForm = document.querySelector('#channels-form');
    let atModemForm = document.querySelector('#at-modem-form');

    let lorawanTemp = document.querySelectorAll('template')[0];   
    let generalTemp = document.querySelectorAll('template')[1];
    let analogTemp = document.querySelectorAll('template')[2];
    let digitalTemp = document.querySelectorAll('template')[3];
    let atModemTemp = document.querySelectorAll('template')[4];    
    
    let formDiv = document.querySelector('#form-div');
    let buttonDiv = document.querySelector('#button-div');
    let lorawanDiv = document.querySelector('#lorawan-div');
    let generalDiv = document.querySelector('#general-div');    
    let analogDiv = document.querySelector('#analog-div');
    let digitalDiv = document.querySelector('#digital-div');
    let atModemDiv = document.querySelector('#at-modem-div');    
     
    let connectBtn = document.querySelector('#connect-btn');    
    let lorawanBtn = document.querySelector('#lorawan-btn');
    let lorawanGetBtn = document.querySelector('#lorawan-get-btn');
    let lorawanSaveBtn = document.querySelector('#lorawan-save-btn');
    let lorawanBackBtn = document.querySelector('#lorawan-back-btn');
    let generalBtn = document.querySelector('#general-btn');
    let generalGetBtn = document.querySelector('#general-get-btn');
    let generalSaveBtn = document.querySelector('#general-save-btn');
    let generalBackBtn = document.querySelector('#general-back-btn');    
    let channelsBtn = document.querySelector('#channels-btn'); 
    let channelsGetBtn = document.querySelector('#channels-get-btn');
    let channelsSaveBtn = document.querySelector('#channels-save-btn');
    let channelsFetchBtn = document.querySelector('#channels-fetch-btn');
    let channelsBackBtn = document.querySelector('#channels-back-btn');
    let atModemBtn = document.querySelector('#at-modem-btn');
    let atModemSendBtn = document.querySelector('#at-modem-send-btn');
    let atModemBackBtn = document.querySelector('#at-modem-back-btn');
    
    let atModemText = document.querySelector('#at-response');

    let port;
    let statusDisp = document.querySelector('#status');

    let numAn = 2, numDg = 2;
    let items;
    
    lorawanBtn.addEventListener('click', function() {
      buttonDiv.hidden = true;
      formDiv.hidden = false;
      lorawanForm.hidden = false;
      generalForm.hidden = true;      
      channelsForm.hidden = true; 
      atModemForm.hidden = true;               
    });

    lorawanGetBtn.addEventListener('click', function() {
      //if (!port) {
      //  return;
      //}      
      //port.send('at+get_config=lora:status' + '\r\n');                 
    });

    lorawanSaveBtn.addEventListener('click', function() {
      //if (!port) {
      //  return;
      //}
      let str = '';               
      if (lorawanForm.checkValidity()) {
        statusDisp.textContent = 'validaion ok';
        items = lorawanForm.querySelectorAll('input,select');                
        for (let i = 0; i < items.length; i++) {          
          str += 'at+set_config=lora:' + items[i].id + ':' + items[i].value + '\r\n';                                           
        }                                         
      }                 
      statusDisp.textContent = str;
      //port.send(str);           
    });

    lorawanBackBtn.addEventListener('click', function() {
      buttonDiv.hidden = false;
      formDiv.hidden = true;                      
    });

    generalBtn.addEventListener('click', function() {
      buttonDiv.hidden = true;
      formDiv.hidden = false;
      lorawanForm.hidden = true;
      generalForm.hidden = false;      
      channelsForm.hidden = true;
      atModemForm.hidden = true;                
    });

    generalGetBtn.addEventListener('click', function() {
      //if (!port) {
      //  return;
      //}      
      //port.send('xget_ge');      
    });

    generalSaveBtn.addEventListener('click', function() {
      //if (!port) {
      //  return;
      //}
      let str = '';          
      if (generalForm.checkValidity()) {
        statusDisp.textContent = 'validaion ok';
        items = generalForm.querySelectorAll('input,select');               
        for (let i = 0; i < items.length; i++) {          
          if (items[i].id[0] == 'x') { 
            str += items[i].id + items[i].value + '\r\n';           
            // str += items[i].id + Number(items[i].value) + '\r\n';                        
          }                                 
        }
        str += 'xsave' + '\r\n';                                 
      }            
      statusDisp.textContent = str;
      //port.send(str);      
    });

    generalBackBtn.addEventListener('click', function() {
      buttonDiv.hidden = false;
      formDiv.hidden = true;                      
    });
    
    channelsBtn.addEventListener('click', function() {
      buttonDiv.hidden = true;
      formDiv.hidden = false;
      lorawanForm.hidden = true;
      generalForm.hidden = true;      
      channelsForm.hidden = false;
      atModemForm.hidden = true;                
    });

    channelsGetBtn.addEventListener('click', function() {
      //if (!port) {
      //  return;
      //}      
      //port.send('xget_ch');      
    });

    channelsSaveBtn.addEventListener('click', function() {
      //if (!port) {
      //  return;
      //}
      let str = '';          
      if (channelsForm.checkValidity()) {
        statusDisp.textContent = 'validaion ok';
        items = channelsForm.querySelectorAll('input,select');
        for (let i = 0; i < items.length; i++) {          
          if (items[i].id[0] == 'x') {             
            str += items[i].id + items[i].value + '\r\n';           
            // str += items[i].id + Number(items[i].value) + '\r\n';                        
          }                                 
        }
        str += 'xsave' + '\r\n';                                 
      }            
      statusDisp.textContent = str;
      //port.send(str);      
    });

    channelsFetchBtn.addEventListener('click', function() {
      //if (!port) {
      //  return;
      //}      
      //port.send('xfetch');      
    });

    channelsBackBtn.addEventListener('click', function() {
      buttonDiv.hidden = false;
      formDiv.hidden = true;                      
    });    
    
    atModemBtn.addEventListener('click', function() {
      buttonDiv.hidden = true;
      formDiv.hidden = false;
      lorawanForm.hidden = true;
      generalForm.hidden = true;      
      channelsForm.hidden = true;
      atModemForm.hidden = false;                
    });    

    atModemSendBtn.addEventListener('click', function() {
      if (!port) {
        return;
      }                    
      if (lorawanForm.checkValidity()) {
        statusDisp.textContent = 'validaion ok';
        //item = atModemForm.querySelector('#at-command');
        //port.send(item.value + '\r\n');
        //port.send('helloworld\r\n');
        let view = new Uint8Array(3);
        view[0] = 1;
        view[1] = 2;
        view[2] = 3;
        port.send(view);                                        
      }                 
    });

    atModemBackBtn.addEventListener('click', function() {
      buttonDiv.hidden = false;
      formDiv.hidden = true;                      
    });
    
    function create() {
      // statusDisp.textContent = '';
      let clon;
      let btns;
      let divs;
      // LORAWAN
      clon = lorawanTemp.content.cloneNode(true);     
      lorawanDiv.appendChild(clon);                     
      // GENERAL
      clon = generalTemp.content.cloneNode(true);     
      generalDiv.appendChild(clon);     
      // ANALOG
      for (let i = 0; i < numAn; i++) {
        clon = analogTemp.content.cloneNode(true);
        analogDiv.appendChild(clon); 
      }      
      btns = analogDiv.querySelectorAll('button');
      divs = analogDiv.querySelectorAll('#analog');
      for (let i = 0; i < numAn; i++) {
        btns[i].setAttribute('data-target', '#analog' + i);
        btns[i].setAttribute('aria-controls', 'analog' + i);        
        btns[i].textContent = '#' + String(i + 1) + ' (Analog ' + String(i + 1) + ')' + ' Value: 0'; 
        divs[i].setAttribute('id', 'analog' + i);      
      } 
      // DIGITAL
      for (let i = 0; i < numDg; i++) {
        clon = digitalTemp.content.cloneNode(true);
        digitalDiv.appendChild(clon); 
      }      
      btns = digitalDiv.querySelectorAll('button');
      divs = digitalDiv.querySelectorAll('#digital');
      for (let i = 0; i < numDg; i++) {
        btns[i].setAttribute('data-target', '#digital' + i);
        btns[i].setAttribute('aria-controls', 'digital' + i);
        btns[i].textContent = '#' + String(i + 1 + numAn) + ' (Digital ' + String(i + 1) + ')' + ' Value: 0'; 
        divs[i].setAttribute('id', 'digital' + i);      
      } 
      // AT MODEM
      clon = atModemTemp.content.cloneNode(true);     
      atModemDiv.appendChild(clon);      
      // make id      
      let datas = 
      ['ge_u08','an_u08','an_u16','an_f32','dg_u08','dg_u16']; 
      for (let i = 0; i < datas.length; i++) {        
        items = formDiv.querySelectorAll('#x' + datas[i]);        
        for (let j = 0; j < items.length; j++) {
          items[j].id += ('0' + j).slice(-2);
          statusDisp.textContent += items[j].id + '\r\n';
        }        
      }
    }
    create();

    function connect() {
      port.connect().then(() => {
        statusDisp.textContent = '';
        connectBtn.textContent = 'Disconnect';
        port.onReceive = data => {
          //let textDecoder = new TextDecoder();
          //console.log(textDecoder.decode(data));
          //statusDisp.textContent += textDecoder.decode(data);
          statusDisp.textContent = 'received';

          /*
          // here readline parser, and trim
          let dataline;
          atModemText.textContent += dataline + '\r\n';         

          let item;
          let value;          
          let split;
          let n = Number(dataline.slice(7, 9));
          let btns = channelsForm.querySelectorAll('button');
          if (dataline.startsWith('xan_val')) {            
            value = dataline.slice(9);
            split = btns[n].textContent.split('Value: ');
            btns[n].textContent = btns[n].textContent.replace(split[1], value);
          } else if (dataline.startsWith('xdg_val')) {            
            value = dataline.slice(9);
            split = btns[n + numAn].textContent.split('Value: ');
            btns[n + numAn].textContent = btns[n + numAn].textContent.replace(split[1], value);                   
          } else {
            if (dataline.startsWith('DevEui: ')) {
              item = loraForm.querySelector('#dev-eui');
              value = dataline.slice(8);                        
            } else if (dataline.startsWith('AppEui: ')) {
              item = loraForm.querySelector('#app-eui');
              value = dataline.slice(8);            
            } else if (dataline.startsWith('AppKey: ')) {
              item = loraForm.querySelector('#app-key');
              value = dataline.slice(8);            
            } else if (dataline.startsWith('x')) {
              item = mainForm.querySelector('#' + dataline.slice(0, 9));
              value = dataline.slice(9);            
            }            
            item.value = value;  
                      
          }
          */
        }
        port.onReceiveError = error => {
          console.error(error);
        };
      }, error => {
         statusDisp.textContent = error;
      });
    }    
    connectBtn.addEventListener('click', function() {
      if (port) {
        port.disconnect();
        connectBtn.textContent = 'Connect';
        statusDisp.textContent = '';
        port = null;
      } else {
        serial.requestPort().then(selectedPort => {
          port = selectedPort;
          connect();
        }).catch(error => {
          statusDisp.textContent = error;
        });
      }
    }); 
    serial.getPorts().then(ports => {
      if (ports.length == 0) {
        statusDisp.textContent = 'No device found.';
      } else {
        statusDisp.textContent = 'Connecting...';
        port = ports[0];
        connect();
      }
    });
    
  });
})();

/*    
serialport.parsers.readline('\n');
parser: SerialPort.parsers.readline('\r')
var serialPort = new SerialPort('/dev/ttyACM0", {
  autoOpen: false,
  parser: SerialPort.parsers.readline('\n'),
  baudrate:115200
}); 

var myPort = new serialport(portName, {
  baudRate: 9600,
  parser: new serialport.parsers.Readline('\r\n')
});
*/

