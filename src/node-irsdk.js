var path = require("path");

const majorVersion = process.version.split(".")[0].replace("v", "");
const platform = process.platform;
const arch = process.arch;

const isElectron = () => {
  // Main process
  if (typeof process !== 'undefined' && typeof process.versions === 'object' && !!process.versions.electron) {
    return true;
  }

  return false;
};

const bindingName = `IrSdkNodeBindings.node`;
if (isElectron()) {
  bindingPath = path.resolve(path.join(__dirname, "../lib/binding", bindingName));
} else {
  bindingPath = path.resolve(
    path.join(
      __dirname,
      "./binding",
      `${majorVersion}-${platform}-${arch}-${bindingName}`
    )
  );
}

var IrSdkNodeWrapper;
try {
  IrSdkNodeWrapper = require(bindingPath);
} catch (err) {
  console.error(
    "Failed to load iRacing SDK bindings.",
    `${majorVersion}-${platform}-${arch} not supported.`,
    err.message
  );
  throw err;
}

var JsIrSdk = require("./JsIrSdk");

/**
  @module irsdk
*/
module.exports = {};

var instance;

/**
  Initialize JsIrSdk, can be done once before using getInstance first time.
  @function
  @static
  @param {Object} [opts] Options
  @param {Integer} [opts.telemetryUpdateInterval=0] Telemetry update interval, milliseconds
  @param {Integer} [opts.sessionInfoUpdateInterval=0] SessionInfo update interval, milliseconds
  @param {iracing~sessionInfoParser} [opts.sessionInfoParser] Custom parser for session info
  @returns {iracing} Running instance of JsIrSdk
  @example
  * var irsdk = require('node-irsdk-2023')
  * // look for telemetry updates only once per 100 ms
  * var iracing = irsdk.init({telemetryUpdateInterval: 100})
*/
var init = (module.exports.init = function (opts) {
  if (!instance) {
    instance = new JsIrSdk(
      IrSdkNodeWrapper,
      Object.assign(
        {
          telemetryUpdateInterval: 0,
          sessionInfoUpdateInterval: 0,
        },
        opts
      )
    );
  }
  return instance;
});

/**
  Get initialized instance of JsIrSdk
  @function
  @static
  @returns {iracing} Running instance of JsIrSdk
  @example
  * var irsdk = require('node-irsdk')
  * var iracing = irsdk.getInstance()
*/
module.exports.getInstance = function () {
  return init();
};
