var fs = require("fs");

function createSessionInfoParser() {
  var yaml = require("js-yaml");

  return function (sessionInfoStr) {
    // Handle empty/whitespace with comma (i.e Abbrevname: , )
    const cleanedSessionInfoStr = sessionInfoStr.replace(
      /^(\s*\w+:\s*),\s*(.*)$/gm,
      "$1'$2'"
    );

    // Handle orphaned negative sign (i.e UserName: - -11)
    const noOrphanedNegative = cleanedSessionInfoStr.replace(
      /^(\s*\w+:\s*)-\s*(-?\d+)$/gm,
      "$1$2"
    );

    var fixedYamlStr = noOrphanedNegative.replace(
      /TeamName: ([^\n]+)/g,
      function (match, p1) {
        if (
          (p1[0] === '"' && p1[p1.length - 1] === '"') ||
          (p1[0] === "'" && p1[p1.length - 1] === "'")
        ) {
          return match; // skip if quoted already
        } else {
          // 2nd replace is unnecessary atm but its here just in case
          return "TeamName: '" + p1.replace(/'/g, "''") + "'";
        }
      }
    );

    const sanitizedSessionInfo = fixedYamlStr.replace(
      /[\x00-\x08\x0B\x0C\x0E-\x1F\x7F\x80-\x9F]/g,
      ""
    );

    // Remove Drivetrain subsection within CarSetup to avoid duplicate 'At' key issues
    const noDrivetrainSessionInfo = sanitizedSessionInfo.replace(
      /^( Drivetrain:\r?\n(?:  .*\r?\n)*)/m,
      ""
    );

    return yaml.load(noDrivetrainSessionInfo);
  };
}

const parseSessionInfo = createSessionInfoParser();

const text = fs.readFileSync("./test.yaml").toString();

const info = parseSessionInfo(text);

console.log(info);
