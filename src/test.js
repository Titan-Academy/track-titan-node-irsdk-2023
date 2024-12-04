const irsdk = require('./node-irsdk');

irsdk.init();

console.log(irsdk.getInstance())
console.log(irsdk.getInstance().sessionInfo)
