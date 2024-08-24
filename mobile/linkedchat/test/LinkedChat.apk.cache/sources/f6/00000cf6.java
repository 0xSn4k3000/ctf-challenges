package com.eiad.linkedchat;

import android.os.Handler;
import android.os.Looper;
import com.eiad.linkedchat.utils;
import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.ObjectInputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.Socket;

/* loaded from: classes3.dex */
public class tunnel {
    public static BufferedReader reader;
    public static Socket socket;
    public static Handler uiHandler;
    public static PrintWriter writer;
    static int PORT = 1337;
    static String IP = "192.168.111.187";

    public static void Init(String ip, int port) throws Exception {
        try {
            socket = new Socket(IP, PORT);
            writer = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()));
            reader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            uiHandler = new Handler(Looper.getMainLooper());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void SendMessage(final String message, final boolean encrypted) {
        Thread sm = new Thread(new Runnable() { // from class: com.eiad.linkedchat.tunnel.1
            @Override // java.lang.Runnable
            public void run() {
                try {
                    if (encrypted) {
                        tunnel.writer.println(utils.encrypt(message));
                        tunnel.writer.flush();
                    } else {
                        tunnel.writer.println(message);
                        tunnel.writer.flush();
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        sm.start();
    }

    public static void ListenForMsgs() {
        Thread sm = new Thread(new Runnable() { // from class: com.eiad.linkedchat.tunnel.2
            @Override // java.lang.Runnable
            public void run() {
                while (true) {
                    try {
                        String msg = tunnel.reader.readLine();
                        if (msg != null) {
                            if (!utils.sym) {
                                String dec_msg = utils.rsaDecrypt(msg, utils.privateKey);
                                if (dec_msg.startsWith("SYKEY")) {
                                    String aesKey = dec_msg.split(" ", 2)[1];
                                    utils.AESKeyString = aesKey.split(":", 2)[0];
                                    utils.AESKeyIVString = aesKey.split(":", 2)[1];
                                    tunnel.SendToUi("[+] Aes key received");
                                    utils.sym = true;
                                    utils.InitAES();
                                    tunnel.SendToUi("[+] Ready");
                                    tunnel.SendToUi("=========================================");
                                }
                            } else {
                                String dec_msg2 = utils.decrypt(msg);
                                System.out.println("----------------------------------");
                                System.out.println(dec_msg2);
                                if (!dec_msg2.startsWith("RESEC")) {
                                    tunnel.SendToUi("Him: " + dec_msg2);
                                } else {
                                    String serObj = dec_msg2.split(": ", 2)[1];
                                    try {
                                        ByteArrayInputStream bis = new ByteArrayInputStream(utils.b64decode(serObj));
                                        ObjectInputStream ois = new ObjectInputStream(bis);
                                        Object obj = ois.readObject();
                                        if (obj instanceof utils.ResecObj) {
                                            utils.ResecObj desObj = (utils.ResecObj) obj;
                                            if (desObj.NewPublicKey.equals(utils.publicKeyString)) {
                                                desObj.changeKeysWithNewOne();
                                                tunnel.SendToUi("[+] Keys updated");
                                                tunnel.SendToUi("=========================================");
                                            }
                                        } else {
                                            System.out.println("Unexpected object type read from Base64 string.");
                                        }
                                    } catch (IOException | ClassNotFoundException e) {
                                        e.printStackTrace();
                                    }
                                }
                            }
                        } else {
                            return;
                        }
                    } catch (Exception e2) {
                        e2.printStackTrace();
                        return;
                    }
                }
            }
        });
        sm.start();
    }

    /* JADX INFO: Access modifiers changed from: private */
    public static void SendToUi(final String msg) {
        uiHandler.post(new Runnable() { // from class: com.eiad.linkedchat.tunnel.3
            @Override // java.lang.Runnable
            public void run() {
                ServerView.addMessage(msg);
            }
        });
    }
}