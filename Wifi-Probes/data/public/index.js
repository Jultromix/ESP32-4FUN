
// Your web app's Firebase configuration
const firebaseConfig = {
apiKey: "your_apiKey",
authDomain: "your_authDomain",
databaseURL: "your_databaseURL",
projectId: "your_projectId",
storageBucket: "your_storageBucket",
messagingSenderId: "your_messagingSenderId",
appId: "your_appid"
};

// Initialize Firebase
firebase.initializeApp(firebaseConfig);

// Sign in with predetermined email and password
const email = "your_email";
const password = "your_password";

// Function to retrieve data from Firebase and download as JSON
const retrieveDataAndDownloadJSON = () => {
  firebase.auth().signInWithEmailAndPassword(email, password)
    .then((userCredential) => {
      // Signed in successfully
      const user = userCredential.user;
      
      // Access Real-Time Database
      const dbRef = firebase.database().ref('/your/path');
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
      const dbRef = firebase.database().ref('/your/path');
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

