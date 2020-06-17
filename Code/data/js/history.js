window.onload = function() {
    let numb = prompt("Введите номер записи: ");
    id.value  = parseInt((numb == null)?1:numb);
    readDatabase(id.value);
}

// База данных
// GET
async function readDatabase(id) {
    // Получение данных
    let response = await fetch("http://192.168.1.15:8000/api/v1/database/logs/detail/" + id);
    if (response.ok || id == 1) {
        let data = await response.json();

        etype.value = data.type;
        userID.value = data.userID;
        door.value = data.door;

        time.value = ((data.hour < 10)?("0" + data.hour):data.hour) + ":" + ((data.minutes < 10)?("0" + data.minutes):data.minutes) + ":" + ((data.seconds < 10)?("0" + data.seconds):data.seconds);
        
        date.value = data.year + "-" + ((data.month < 10)?("0" + data.month):data.month) + "-" + ((data.day < 10)?("0" + data.day):data.day);
    }
    else {
        document.getElementById("id").value = 1;
        id = 1;
        
        readDatabase(id);
        alert("Запись удалена, либо пуста.");
    };
};

// DELETE
async function deleteEntry() {
    var requestOptions = {
        method: 'DELETE',
        redirect: 'follow'
    };
      
    fetch("http://192.168.1.15:8000/api/v1/database/logs/detail/" + id.value, requestOptions)
        .then(response => response.text())
        .then(result => console.log(result))
        .catch(error => console.log('error', error));

    readDatabase(id.value);
}