const net = require('net');
const fs = require('fs');

const PORT = 6435;
const HOST = '127.0.0.1';
const client = new net.Socket();



function connectToServerAndReceiveData() {
    console.time('Execution Time');

        // Connect to the server and handle communication in the callback
        client.connect(PORT, HOST, () => {
        
        
            // Send an empty request to trigger the data transfer
            client.write('');
        });


    // Handle incoming data from the server
    client.on('data', (data) => {
        console.log('Data received from the server.');
        
        // Process the received data, here we are just printing it
        const xp = data.toString('utf8');
        

        
        

    });
    console.timeEnd('Execution Time');



}


// Run the function to connect to the server and receive data
connectToServerAndReceiveData();


    // Handle connection closure
    client.on('close', () => {
        console.log('Connection closed');
        
    });