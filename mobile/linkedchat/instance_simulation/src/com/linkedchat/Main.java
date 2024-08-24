package com.linkedchat;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.ServerSocket;
import java.net.Socket;

import java.security.KeyFactory;
import java.security.PublicKey;
import java.security.spec.X509EncodedKeySpec;
import java.util.Base64;


import javax.crypto.Cipher;
import javax.crypto.NoSuchPaddingException;

import javax.crypto.KeyGenerator;
import javax.crypto.spec.SecretKeySpec;
import javax.crypto.spec.IvParameterSpec;
import java.security.SecureRandom;

import java.util.HashMap;
import java.util.Map;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

import java.io.ByteArrayInputStream;
import java.io.ObjectInputStream;
import java.io.ByteArrayOutputStream;
import java.io.ObjectOutputStream;


public class Main {

    private static Map<Socket, PublicKey> clients = new HashMap<>();
    private static Map<Socket, BufferedWriter> clientsWriters = new HashMap<>();

    private static String SymKey;
    private static String AdminKey = "SecretSecret";

    private static IvParameterSpec aesIv, oldAesIv;
    private static SecretKeySpec aesKey, oldAesKey;

    public static String b64encode(byte[] data) {
        return Base64.getEncoder().encodeToString(data);
    }

    public static byte[] b64decode(String base64) {
        return Base64.getDecoder().decode(base64);
    }


    public static PublicKey convertPublicKey(String publicKeyBase64String) throws Exception {
        byte[] publicKeyBytes = b64decode(publicKeyBase64String);

        KeyFactory keyFactory = KeyFactory.getInstance("RSA");
        X509EncodedKeySpec keySpec = new X509EncodedKeySpec(publicKeyBytes);
        return keyFactory.generatePublic(keySpec);
    }

    public static String rsaEncrypt(String plainText, PublicKey Key) throws Exception {
        Cipher cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
        cipher.init(Cipher.ENCRYPT_MODE, Key);
        byte[] cipherBytes = cipher.doFinal(plainText.getBytes());
        return b64encode(cipherBytes);
    }


    public static void GenerateAesKey() {
        try {
            KeyGenerator keyGen = KeyGenerator.getInstance("AES");
            keyGen.init(256);
            aesKey = new SecretKeySpec(keyGen.generateKey().getEncoded(), "AES");

            SecureRandom random = new SecureRandom();
            byte[] ivBytes = new byte[16];
            random.nextBytes(ivBytes);
            aesIv = new IvParameterSpec(ivBytes);

            SymKey = b64encode(aesKey.getEncoded()) + ":" + b64encode(ivBytes);
            System.out.println("[+] Aes key: " + SymKey);
        } catch(Exception e) {
            System.out.println(e.getMessage());
        }
    }


    public static String encrypt(String plainText, SecretKeySpec key, IvParameterSpec iv) {
        try {
            Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5PADDING");
            cipher.init(Cipher.ENCRYPT_MODE, key, iv);

            byte[] encryptedData = cipher.doFinal(plainText.getBytes());

            return new String(b64encode(encryptedData));
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

    public static String decrypt(String encryptedText, SecretKeySpec key, IvParameterSpec iv) {
        try {
            Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5PADDING");
            cipher.init(Cipher.DECRYPT_MODE, key, iv);

            byte[] decodedEncryptedData = b64decode(encryptedText);
            byte[] decryptedData = cipher.doFinal(decodedEncryptedData);

            return new String(decryptedData);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

    public static void main(String[] args) {
        final int portNumber = 1337; 


        try {
            ServerSocket serverSocket = new ServerSocket(portNumber);
            System.out.println("Server started. Listening on port " + portNumber + "...");
           
            GenerateAesKey();

            while (true) {
                Socket clientSocket = serverSocket.accept();
                System.out.println("Client connected: " + clientSocket.getInetAddress().getHostAddress());
                
                Thread clientThread = new Thread(() -> {
                    try {
                        BufferedReader reader = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
                        BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(clientSocket.getOutputStream()));

                        clientsWriters.put(clientSocket, writer);
                        String inputLine, pubkeystr;
                        PublicKey pubkey;
                        while ((inputLine = reader.readLine()) != null) {
                            if(inputLine.startsWith("PUKEY")) {
                                pubkeystr = inputLine.split(": ", 2)[1];
                                System.out.println("---------------------------");
                                System.out.println(clientSocket.getInetAddress().getHostAddress() + " Public Key: " + pubkeystr);
                                System.out.println("---------------------------");
                                try {
                                    clients.put(clientSocket, convertPublicKey(pubkeystr));
                                   
                                    String enc = rsaEncrypt("SYKEY: " + SymKey, clients.get(clientSocket));
                                    
                                    writer.write(enc);
                                    writer.newLine();
                                    writer.flush();

                                } catch(Exception e) {
                                    System.err.println(e.getMessage());
                                }


                            } else {
                                String msg = decrypt(inputLine, aesKey, aesIv);
                                
                                if (msg.startsWith("RESER: ")) {
                                    System.out.println("[+] resecure request");
                                   
                                    String serObj = msg.split(": ", 2)[1];

                                    try {
                                        ByteArrayInputStream bis = new ByteArrayInputStream(b64decode(serObj));
                                        ObjectInputStream ois = new ObjectInputStream(bis);
                                        Object obj = ois.readObject();

                                        utils.ResecObj desObj = (utils.ResecObj) obj;

                                        System.out.println("After serialization");
                                        oldAesKey = aesKey;
                                        oldAesIv = aesIv;
                                        GenerateAesKey();

                                        clients.put(clientSocket, convertPublicKey(desObj.NewPublicKey));
                                        
                                        clientsWriters.forEach((socket, socketWriter) -> {
                                            
                                            if(!socket.equals(clientSocket)){
                                                try { 
                                                    socketWriter.write(encrypt("RESEC: " + SymKey, oldAesKey, oldAesIv));
                                                    socketWriter.newLine();
                                                    socketWriter.flush();

                                                } catch (Exception e) {
                                                    System.out.println(e.getMessage());
                                                }
                                            } else {
                                                try {
                                                    socketWriter.write(rsaEncrypt("SYKEY: " + SymKey, clients.get(socket)));
                                                    socketWriter.newLine();
                                                    socketWriter.flush();
                                                } catch (Exception e) {
                                                    System.out.println(e.getMessage());
                                                }
                                            }
                                        });
                        
                                    } catch (Exception e) {
                                        System.out.println(e.getMessage());
                                    }

                                } else if(msg.startsWith("EXECA: ")) {
                                    String serObj = msg.split(": ", 2)[1];

                                    try {
                                        ByteArrayInputStream bis = new ByteArrayInputStream(b64decode(serObj));
                                        ObjectInputStream ois = new ObjectInputStream(bis);
                                        Object obj = ois.readObject();

                                        utils.ExecObj desObj = (utils.ExecObj) obj;

                                        if(desObj.Key == AdminKey) {
                                            System.out.println("true");
                                        }
                                    } catch(Exception e) {
                                        System.out.println(e.getMessage());
                                    }


                                } else {
                                    final String FMsg = inputLine;
                                    clientsWriters.forEach((socket, socketWriter) -> {
                                        if(!socket.equals(clientSocket)){
                                            try { 
                                                socketWriter.write(FMsg);
                                                socketWriter.newLine();
                                                socketWriter.flush();

                                            } catch (IOException e) {
                                                System.out.println(e.getMessage());
                                            }
                                        }
                                    });
                                    
                                }

                            }
                        }
                        
                        writer.close();
                        reader.close();
                        clientSocket.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                });
                
                clientThread.start();
            }
            
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}

