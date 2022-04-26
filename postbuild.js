console.log("Start");
console.log(process.argv[2]);
if (process.argv[2] == "no-upload") {
    console.log("No upload requested. Returning...")
    process.exit();
}


const root = 'C:\\Users\\Papi\\AppData\\Roaming\\npm\\node_modules';
const fs = require('fs');


const version = getFirstLine('src/vivarium/version.h').split(" ")[2].replace(/\"/g, '');
if (version) {
    console.log("Version: " + version);
} else {
    console.error("No version set");
    process.exit();
}



function getFirstLine(filePath) {
    const fileContent = fs.readFileSync(filePath, 'utf-8')
    return (fileContent.match(/(^.*)/) || [])[1] || '';
}


const filename = 'vivarium-control-aquarium.ino_' + version + '.bin';
console.log('Version = ' + version);



/// Init firebase
console.log('init firebase');
const admin = require(`${root}/firebase-admin`);
const serviceAccount = require('./data/service_account_file.json');


admin.initializeApp({
    credential: admin.credential.cert(serviceAccount),
    storageBucket: "vivarium-control-unit.appspot.com",
    databaseURL: "https://vivarium-control-unit.firebaseio.com"
});

const db = admin.database();
const ref = db.ref('firmware');
var bucket = admin.storage().bucket();

upload();
copyBuild();
async function upload() {
    /// Upload firmware
    console.log("Upload firmware with name" + filename)
    await bucket.upload('./build/vivarium_esp.ino.bin', {
        destination: 'firmware/' + filename,
    });

    /// Update DB
    console.log("Update DB");

    let comment = ""
    if (process.argv.length >= 4) {
        comment = process.argv[3]
    }

    await ref.update({
        [version]: {
            'filename': filename,
            'comment': comment
        }
    });

    console.log('Finished');
    process.exit();
}


function copyBuild() {
    fs.copyFile('build/vivarium_esp.ino.elf', 'elf/vivarium_esp_' + version + ".elf", (err) => {
        if (err) throw err;
        console.log('ELF  was copied to ./elf/vivarium_esp_' + version + ".elf");
    });

}