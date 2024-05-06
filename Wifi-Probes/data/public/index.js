
// Your web app's Firebase configuration
const firebaseConfig = {
  apiKey: "AIzaSyANRE24xIvVL1zOJZ_aLpTgbstZtTvJipg",
  authDomain: "esp324fun-bb666.firebaseapp.com",
  databaseURL: "https://esp324fun-bb666-default-rtdb.firebaseio.com",
  projectId: "esp324fun-bb666",
  storageBucket: "esp324fun-bb666.appspot.com",
  messagingSenderId: "361944136379",
  appId: "1:361944136379:web:ed15287b583444ba8c879e"
  };

// Initialize Firebase
firebase.initializeApp(firebaseConfig);

// Sign in with predetermined email and password
const email = "esp324fun@gmail.com";
const password = "$_Esp324fun_psswd";




// Function to retrieve data from Firebase and download as JSON
const retrieveDataAndDownloadJSON = () => {
  firebase.auth().signInWithEmailAndPassword(email, password)
    .then((userCredential) => {
      // Signed in successfully
      const user = userCredential.user;
      
      // Access Real-Time Database
      const dbRef = firebase.database().ref('/UsersData/ZydQXP62DyXKo3kSYFivzxSb89S2/readings');
      dbRef.once('value', (snapshot) => {
        const data = snapshot.val();
        
        // Convert data to JSON string
        const jsonString = JSON.stringify(data, null, 2);
        
        // Create Blob object
        const blob = new Blob([jsonString], { type: "application/json" });
        
        // Create download link
        const downloadLink = document.createElement("a");
        downloadLink.href = URL.createObjectURL(blob);
        downloadLink.download = "data.json";
        downloadLink.innerHTML = "Download JSON";
        
        // Append download link to the DOM
        document.body.appendChild(downloadLink);
        
        // Trigger download
        downloadLink.click();
        
        // Remove download link from the DOM
        document.body.removeChild(downloadLink);
      });
    })
    .catch((error) => {
      console.error("Authentication failed:", error);
    });
};
// Add click event listener to the download button
document.getElementById("downloadJson").addEventListener("click", retrieveDataAndDownloadJSON);


// Function to retrieve data from Firebase and download as CSV
const retrieveDataAndDownloadCSV = () => {
  firebase.auth().signInWithEmailAndPassword(email, password)
    .then((userCredential) => {
      // Signed in successfully
      const user = userCredential.user;
      
      // Access Real-Time Database
      const dbRef = firebase.database().ref('/UsersData/ZydQXP62DyXKo3kSYFivzxSb89S2/readings');
      dbRef.once('value', (snapshot) => {
        const data = snapshot.val();
        
        // Convert data to CSV string
        let csvString = "Key,Values\n";
        for (const key in data) {
          if (data.hasOwnProperty(key)) {
            const values = Object.values(data[key]);
            csvString += `${key},"${values.join(",")}"\n`;
          }
        }
        
        // Create Blob object
        const blob = new Blob([csvString], { type: "text/csv" });
        
        // Create download link
        const downloadLink = document.createElement("a");
        downloadLink.href = URL.createObjectURL(blob);
        downloadLink.download = "data.csv";
        downloadLink.innerHTML = "Download CSV";
        
        // Append download link to the DOM
        document.body.appendChild(downloadLink);
        
        // Trigger download
        downloadLink.click();
        
        // Remove download link from the DOM
        document.body.removeChild(downloadLink);
      });
    })
    .catch((error) => {
      console.error("Authentication failed:", error);
    });
};
// Add click event listener to the download button
document.getElementById("downloadCsv").addEventListener("click", retrieveDataAndDownloadCSV);


//Update monitor on each new reading
firebase.auth().signInWithEmailAndPassword(email, password)
  .then((userCredential) => {
      // Signed in successfully
      const user = userCredential.user;
      
      // Access Real-Time Database
      const dbRef = firebase.database().ref('/UsersData/ZydQXP62DyXKo3kSYFivzxSb89S2/readings');
      dbRef.on('value', (snapshot) => {
        const data = snapshot.val();
        const subkeyValues = {};
        for (const timestampKey in data) {
          if (data.hasOwnProperty(timestampKey)) {
            for (const subkey in data[timestampKey]) {
              if (data[timestampKey].hasOwnProperty(subkey)) {
                const value = data[timestampKey][subkey];
                const path = `/UsersData/ZydQXP62DyXKo3kSYFivzxSb89S2/readings/${timestampKey}/${subkey}`;
                subkeyValues[subkey] = { value, path };
              }
            }
          }
        }
      // Now subkeyValues object contains each subkey along with its value and path
      // console.log(subkeyValues);

      // Convert the subkeyValues object to an array (you also have the dynamic paths, but won't be needed)
      const subkeyArray = Object.values(subkeyValues);
      // console.log(subkeyArray[0].value, 0); this was to see which values was I reading in the monitor
      // console.log(subkeyArray[1].value, 1);
      // console.log(subkeyArray[2].value, 2);
      // console.log(subkeyArray[3].value, 3);
      // console.log(subkeyArray[4].value, 4);
      // console.log(subkeyArray[5].value, 5);
      // console.log(subkeyArray[6].value, 6);
      // console.log(subkeyArray[7].value, 7);
      // console.log(subkeyArray[8].value, 8);
      
      const temp1Element = document.getElementById("tempsensor1");
      const temp2Element = document.getElementById("tempsensor2");
      const temp3Element = document.getElementById("tempsensor3");
      const temp4Element = document.getElementById("tempsensor4");
      const hum1Element = document.getElementById("humsensor1");
      const hum2Element = document.getElementById("humsensor2");
      const hum3Element = document.getElementById("humsensor3");
      const gas1Element = document.getElementById("gassensor1");
      const gas2Element = document.getElementById("gassensor2");

      gas1Element.innerText =subkeyArray[0].value;
      gas2Element.innerText =subkeyArray[1].value;
      hum1Element.innerText = subkeyArray[2].value;
      hum2Element.innerText = subkeyArray[3].value;
      hum3Element.innerText = subkeyArray[4].value;
      temp1Element.innerText = subkeyArray[5].value;
      temp2Element.innerText = subkeyArray[6].value;
      temp3Element.innerText = subkeyArray[7].value;
      temp4Element.innerText = subkeyArray[8].value;

      });
    })
.catch((error) => {
  console.error("Authentication failed:", error);
});