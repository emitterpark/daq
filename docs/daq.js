(function() {
  'use strict';

  document.addEventListener('DOMContentLoaded', event => {
    let lorawanForm = document.querySelector('#lorawan-form');
    let channelsForm = document.querySelector('#channels-form');
    let atModemForm = document.querySelector('#at-modem-form');

    let lorawanTemp = document.querySelectorAll('template')[0];   
    let analogTemp = document.querySelectorAll('template')[1];
    let digitalTemp = document.querySelectorAll('template')[2];
    let atModemTemp = document.querySelectorAll('template')[3];    
    
    let headerDiv = document.querySelector('#header-div');
    let formDiv = document.querySelector('#form-div');
    let buttonDiv = document.querySelector('#button-div');
    let lorawanDiv = document.querySelector('#lorawan-div');
    let analogDiv = document.querySelector('#analog-div');
    let digitalDiv = document.querySelector('#digital-div');
    let atModemDiv = document.querySelector('#at-modem-div');    
     
    let connectBtn = document.querySelector('#connect-btn');    
    let lorawanBtn = document.querySelector('#lorawan-btn');
    let lorawanSaveBtn = document.querySelector('#lorawan-save-btn');
    let lorawanBackBtn = document.querySelector('#lorawan-back-btn');
    let channelsBtn = document.querySelector('#channels-btn');
    let channelsReportBtn = document.querySelector('#channels-report-btn'); 
    let channelsSaveBtn = document.querySelector('#channels-save-btn');    
    let channelsBackBtn = document.querySelector('#channels-back-btn');
    let atModemBtn = document.querySelector('#at-modem-btn');
    let atModemSendBtn = document.querySelector('#at-modem-send-btn');
    let atModemBackBtn = document.querySelector('#at-modem-back-btn');    
        
    let deviceDisp = document.querySelector('#device');
    let versionDisp = document.querySelector('#version');
    let joinloraDisp = document.querySelector('#joinlora');
    let port;

    let numAn = 2, numDg = 2;
    let item;
    let items; 
    
    let lorawanBuf = [];

    setInterval(function(){ 
      if (!port) {
        return;
      }
      if (lorawanBuf.length > 0) {
        const encoder = new TextEncoder();
        let view;
        view = encoder.encode(lorawanBuf[0]);
        lorawanBuf.shift();
        port.send(view);        
      }
    }, 1000);

    formDiv.addEventListener('click', function() {
      this.scrollIntoView();               
    });  
    
    lorawanBtn.addEventListener('click', function() {
      headerDiv.hidden = true;
      buttonDiv.hidden = true;
      formDiv.hidden = false;
      lorawanForm.hidden = false;
      channelsForm.hidden = true; 
      atModemForm.hidden = true;               
    });    

    lorawanSaveBtn.addEventListener('click', function() {
      if (!port) {
        return;
      }
      const encoder = new TextEncoder();
      let view;
      let str = '';               
      if (lorawanForm.checkValidity()) {
        items = lorawanForm.querySelectorAll('input,select');                
        for (let i = 0; i < items.length; i++) {          
          if (items[i].id[0] == 'x') {           
            str += items[i].id + items[i].value + '\r\n';                                     
          } else {
            lorawanBuf.push('at+set_config=lora:' + items[i].id + ':' + items[i].value + '\r\n');            
          }                                 
        }
        view = encoder.encode(str);      
        port.send(view);
        lorawanBuf.push('xsave\r\n');
        /*
        str += 'xsave\r\n';
        view = encoder.encode(str);      
        port.send(view);
        */
      } 
    });

    lorawanBackBtn.addEventListener('click', function() {
      headerDiv.hidden = false;
      buttonDiv.hidden = false;
      formDiv.hidden = true;                      
    });
    
    channelsBtn.addEventListener('click', function() {
      headerDiv.hidden = true;
      buttonDiv.hidden = true;
      formDiv.hidden = false;
      lorawanForm.hidden = true;           
      channelsForm.hidden = false;
      atModemForm.hidden = true;                
    });  
    
    channelsReportBtn.addEventListener('click', function() {
      if (!port) {
        return;
      }
      const encoder = new TextEncoder();
      let view;                  
      view = encoder.encode('xreport\r\n');                
      port.send(view);
    });  

    channelsSaveBtn.addEventListener('click', function() {
      if (!port) {
        return;
      }
      const encoder = new TextEncoder();
      let view;
      let str = '';          
      if (channelsForm.checkValidity()) {
        items = channelsForm.querySelectorAll('input,select');
        for (let i = 0; i < items.length; i++) {          
          if (items[i].id[0] == 'x') {             
            str += items[i].id + items[i].value + '\r\n';                                  
          }                                 
        }
        str += 'xsave' + '\r\n';        
        view = encoder.encode(str);      
        port.send(view);                                
      }             
    });    

    channelsBackBtn.addEventListener('click', function() {
      headerDiv.hidden = false;
      buttonDiv.hidden = false;
      formDiv.hidden = true;                      
    });    
    
    atModemBtn.addEventListener('click', function() {
      headerDiv.hidden = true;
      buttonDiv.hidden = true;
      formDiv.hidden = false;
      lorawanForm.hidden = true;            
      channelsForm.hidden = true;
      atModemForm.hidden = false;                
    });    

    atModemSendBtn.addEventListener('click', function() {
      if (!port) {
        return;
      }
      const encoder = new TextEncoder();
      let view;                    
      if (lorawanForm.checkValidity()) {
        item = atModemForm.querySelector('#at-command');        
        view = encoder.encode(item.value + '\r\n');                
        port.send(view);
      }                 
    });

    atModemBackBtn.addEventListener('click', function() {
      headerDiv.hidden = false;
      buttonDiv.hidden = false;
      formDiv.hidden = true;                      
    });
    
    function create() {           
      let clon;
      let btns;
      let divs;
      // LORAWAN
      clon = lorawanTemp.content.cloneNode(true);     
      lorawanDiv.appendChild(clon);           
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
        btns[i].textContent = '#' + String(i + 1) + ' (Analog ' + String(i + 1) + ')' + ' : '; 
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
        btns[i].textContent = '#' + String(i + 1 + numAn) + ' (Digital ' + String(i + 1) + ')' + ' : '; 
        divs[i].setAttribute('id', 'digital' + i);      
      } 
      // AT MODEM
      clon = atModemTemp.content.cloneNode(true);     
      atModemDiv.appendChild(clon);      
      // make id      
      let datas = 
      ['lru08','anu08','anu16','anf32','dgu08','dgu16']; 
      for (let i = 0; i < datas.length; i++) {        
        items = formDiv.querySelectorAll('#x' + datas[i]);        
        for (let j = 0; j < items.length; j++) {
          items[j].id += ('0' + j).slice(-2);          
        }        
      }
    }
    create();
    let atModemText = document.querySelector('#at-response');
    atModemText.value = '';

    function connect() {
      port.connect().then(() => {
        deviceDisp.textContent = 'Connected.';
        connectBtn.textContent = 'DISCONNECT';        
        port.onReceive = data => {
          let textDecoder = new TextDecoder();
          let dataline = textDecoder.decode(data);
          atModemText.value += dataline;
          dataline = dataline.trim(); 
          let value = dataline.slice(8);
          let btns = channelsForm.querySelectorAll('button');         
          if (dataline.startsWith('xanval')) {            
            let n = Number(dataline.slice(6, 8));                        
            btns[n].textContent = btns[n].textContent.slice(0, 16);
            btns[n].textContent += value;
          } else if (dataline.startsWith('xdgval')) {
            let n = Number(dataline.slice(6, 8));
            btns[n + numAn].textContent = btns[n + numAn].textContent.slice(0, 17);
            btns[n + numAn].textContent += parseInt(value) ? 'HIGH' : 'LOW';          
          } else if (dataline.startsWith('xdevicee')) {
            deviceDisp.textContent = value;
          } else if (dataline.startsWith('xversion')) {
            versionDisp.textContent = value;
          } else if (dataline.startsWith('xjoinlora')) {
            joinloraDisp.textContent = value; 
          } else if (dataline.startsWith('x')) {
            item = formDiv.querySelector('#' + dataline.slice(0, 8));
            item.value = value;                             
          } else if (dataline.startsWith('DevEui: ')) {
            item = lorawanForm.querySelector('#dev_eui');
            item.value = value;                        
          } else if (dataline.startsWith('AppEui: ')) {
            item = lorawanForm.querySelector('#app_eui');
            item.value = value;            
          } else if (dataline.startsWith('AppKey: ')) {
            item = lorawanForm.querySelector('#app_key');
            item.value = value;
          } else if (dataline.startsWith('Region: ')) {
            item = lorawanForm.querySelector('#region');
            item.value = value;                  
          }                                              
        }
        port.onReceiveError = error => {
          //console.error(error);
        };
        /*
        const encoder = new TextEncoder();
        let view;
        view = encoder.encode('xdevice\r\n');      
        port.send(view);
        view = encoder.encode('xversion\r\n');      
        port.send(view);
        view = encoder.encode('xfetch\r\n');      
        port.send(view);
        view = encoder.encode('xchannels\r\n');      
        port.send(view);
        view = encoder.encode('xlorawan\r\n');      
        port.send(view);
        view = encoder.encode('at+get_config=lora:status\r\n');      
        port.send(view);
        */        
      }, error => {
         deviceDisp.textContent = error;
         versionDisp.textContent = '';
         joinloraDisp.textContent = '';
      });
    }    
    connectBtn.addEventListener('click', function() {
      if (port) {
        port.disconnect();
        connectBtn.textContent = 'CONNECT';
        deviceDisp.textContent = '';
        versionDisp.textContent = '';
        joinloraDisp.textContent = '';
        port = null;
      } else {
        serial.requestPort().then(selectedPort => {
          port = selectedPort;
          connect();
        }).catch(error => {
          deviceDisp.textContent = error;
          versionDisp.textContent = '';
          joinloraDisp.textContent = '';
        });
      }
    }); 
    serial.getPorts().then(ports => {
      if (ports.length == 0) {
        deviceDisp.textContent = 'No device found.';
        versionDisp.textContent = '';
        joinloraDisp.textContent = '';
      } else {
        deviceDisp.textContent = 'Connecting...';
        versionDisp.textContent = '';
        joinloraDisp.textContent = '';
        port = ports[0];
        connect();
      }
    });
    
  });
})();
