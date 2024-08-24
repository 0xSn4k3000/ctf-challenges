Java.perform(function(){
    var utils = Java.use("com.linkedchat.utils");
    console.log("[*] Hooking isDeviceRooted...");
    utils.isDeviceRooted.implementation = function() {
        console.log("[+] Hooked.");
        return false;
    }
})
