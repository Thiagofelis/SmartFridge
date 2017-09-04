

socket = new WebSocket("ws://" + window.location.host + "/chat/");
socket.onmessage = function(e) {
    Notify ("Temperatura Atingida", "Meus parabens")
}
function loadDoc() {
    socket.send("hello world");
}

function valForm( form ){
   // if (int(form.temperatura.value) <= -40 || int(form.temperatura.value) >= 40)
    //{
     //   alert ("e a temp")
   // }
    if (document.getElementById('lataa').checked)
    {

    }
    if (document.getElementById('latab').checked)
    {

    }

}

// Determine the correct object to use
var notification = window.Notification || window.mozNotification || window.webkitNotification;

// The user needs to allow this
if ('undefined' === typeof notification)
    alert('Web notification not supported');
else
    notification.requestPermission(function(permission){});

// A function handler
function Notify(titleText, bodyText)
{
    if ('undefined' === typeof notification)
        return false;       //Not supported....
    var noty = new notification(
        titleText, {
            body: bodyText,
            dir: 'auto', // or ltr, rtl
            lang: 'EN', //lang used within the notification.
            tag: 'notificationPopup', //An element ID to get/set the content
            icon: 'https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcSOtmX64mfCwygW_oE9Wf9PFKLUn8BW3sL-6l-fAedtvpixn0B0'
        }
    );
    noty.onclick = function () {
        console.log('notification.Click');
    };
    noty.onerror = function () {
        console.log('notification.Error');
    };
    noty.onshow = function () {
        console.log('notification.Show');
    };
    noty.onclose = function () {
        console.log('notification.Close');
    };
    return true;
}