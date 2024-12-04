const fs = require('fs');
const path = require('path');
const os = require('os');

function copyBindings() {
  const srcDir = path.join(__dirname, 'lib', 'binding');
  const nodeMajorVersion = process.versions.node.split('.')[0];
  const platform = os.platform();
  const arch = os.arch();
  const bindingName = `IrSdkNodeBindings.node`;

  fs.readdir(srcDir, (err, files) => {
    if (err) {
      console.error(`Error reading source directory: ${err}`);
      return;
    }

    files.forEach(file => {
      const srcFile = path.join(srcDir, file);
      const destFileName = `${nodeMajorVersion}-${platform}-${arch}-${bindingName}`;
      const destFile = path.join(__dirname, 'binding', destFileName);

      fs.copyFile(srcFile, destFile, err => {
        if (err) {
          console.error(`Error copying file ${file}: ${err}`);
        } else {
          console.log(`Copied ${file} to ${destFile}`);
        }
      });
    });
  });
}

copyBindings();
