const irsdk = require('./node-irsdk');

irsdk.init();

console.log(irsdk)
console.log(irsdk.getInstance().sessionInfo)
