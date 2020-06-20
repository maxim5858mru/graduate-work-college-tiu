window.onload = function() {
    let numb = prompt("Введите номер записи: ");
    id.value  = parseInt((numb == null)?0:numb);
    readDatabase(id.value);
}

// База данных
// GET
async function readDatabase(id) {
    // Получение данных
    let response = await fetch("http://192.168.1.15:8000/api/v1/database/users/detail/" + id);
    if (response.ok) {
        let data = await response.json();

        fname.value = data.firstName;
        lname.value = data.lastName;
        mname.value = data.middleName;
        door.value = (data.door == 0 || data.door == -1)?"":data.door;
        mPIN.checked = data.methodPIN;
        mRFID.checked = data.methodRFID;
        mFP.checked = data.methodFPID;
        pin.value = (data.pin == 0 || data.pin == -1)?"":data.pin;
        fp.value = (data.fingerprintID == 0 || data.fingerprintID == -1)?"":data.fingerprintID;

        let rfid_block = document.getElementsByClassName("rfid_data");
        rfid_block[0].value = data.rfid1;
        rfid_block[1].value = data.rfid2;
        rfid_block[2].value = data.rfid3;
        rfid_block[3].value = data.rfid4;
        rfid_block[4].value = data.rfid5;
        rfid_block[5].value = data.rfid6;
        rfid_block[6].value = data.rfid7;
        rfid_block[7].value = data.rfid8;
        rfid_block[8].value = data.rfid9;
        rfid_block[9].value = data.rfid10;

        let sMin = data.startTime % 100;
        let sHour = data.startTime / 100;
        sMin = sMin < 10?("0" + sMin):(sMin); 
        sHour = sHour < 10?("0" + sHour):(sHour); 
        stime.value = sHour + ":" + sMin; 

        let eMin = data.endTime % 100;
        let eHour = data.endTime / 100;
        eMin = eMin < 10?("0" + eMin):(eMin); 
        eHour = eHour < 10?("0" + eHour):(eHour); 
        etime.value = eHour + ":" + eMin; 
    }
    else if (id == 0) {
        console.log("Ошибка HTTP: " + response.status);

        fname.value = "";
        lname.value = "";
        mname.value = "";
        door.value = "";
        mPIN.checked = false;
        mRFID.checked = false;
        mFP.checked = false;
        pin.value = "";
        fp.value = "";

        let rfid_block = document.getElementsByClassName("rfid_data");

        for(var i = 0; i <  rfid_block.length; i++) {
            rfid_block[i].value = "";
        }

        stime.value = "00:00"; 
        etime.value = "00:00"; 
    }
    else {
        document.getElementById("id").value = 0;
        id = 0;
        
        readDatabase(id);
        alert("Запись удалена либо пуста.");
    };
    
    // Изменение формы в соотвестии с данными
    changePIN();
    changeFP();
    changeFP();
};

// DELETE
async function deleteEntry() {
    var requestOptions = {
        method: 'DELETE',
        redirect: 'follow'
    };
      
    fetch("http://192.168.1.15:8000/api/v1/database/users/detail/" + id.value, requestOptions)
        .then(response => response.text())
        .then(result => console.log(result))
        .catch(error => console.log('error', error));

    readDatabase(id.value);
}

// PUT & POST
async function putEntry() {
    let rfid_block = document.getElementsByClassName("rfid_data");
    let data = {
        "id": parseInt(id.value),
        "firstName": fname.value,
        "lastName": lname.value,
        "middleName": mname.value,
        "door": parseInt((door.value != "")?door.value:0),
        "methodPIN": mPIN.checked,
        "methodRFID": mRFID.checked,
        "methodFPID": mFP.checked,
        "pin": parseInt((pin.value != "")?pin.value:0),
        "fingerprintID": parseInt((fp.value != "")?fp.value:0),
        "rfid1": parseInt((rfid_block[0].value != "")?rfid_block[0]:0),
        "rfid2": parseInt((rfid_block[1].value != "")?rfid_block[1]:0),
        "rfid3": parseInt((rfid_block[2].value != "")?rfid_block[2]:0),
        "rfid4": parseInt((rfid_block[3].value != "")?rfid_block[3]:0),
        "rfid5": parseInt((rfid_block[4].value != "")?rfid_block[4]:0),
        "rfid6": parseInt((rfid_block[5].value != "")?rfid_block[5]:0),
        "rfid7": parseInt((rfid_block[6].value != "")?rfid_block[6]:0),
        "rfid8": parseInt((rfid_block[7].value != "")?rfid_block[7]:0),
        "rfid9": parseInt((rfid_block[8].value != "")?rfid_block[8]:0),
        "rfid10": parseInt((rfid_block[9].value != "")?rfid_block[9]:0),
        "startTime": parseInt(stime.value.replace(":", "")),
        "endTime": parseInt(etime.value.replace(":", "")),
        "user": 1
    };

    if (id.value != 0) {
        let response = await fetch('http://192.168.1.15:8000/api/v1/database/users/detail/' + id.value, {
            method: 'PUT',
            headers: {
              'Content-Type': 'application/json;charset=utf-8'
            },
            body: JSON.stringify(data)
        });
    }
    else {
        delete data.id; 

        let response = await fetch('http://192.168.1.15:8000/api/v1/database/users/create', {
            method: 'POST',
            headers: {
              'Content-Type': 'application/json;charset=utf-8'
            },
            body: JSON.stringify(data)
        });

        if (id.value == 0) {
            let result = await response.json();
            document.getElementById("id").value = result.id;
        }
    }

    alert("Сохранено");
}

// PIN
function changePIN() {
    pin.readOnly = !mPIN.checked;
}

// RFID
function changeRFID() {
    let rfid_block = document.getElementsByClassName("rfid_data");

    for(var i = 0; i <  rfid_block.length; i++) {
        rfid_block[i].readOnly = !mRFID.checked;
    };
}

function changeFP() {
    fp.readOnly = !mFP.checked;
}