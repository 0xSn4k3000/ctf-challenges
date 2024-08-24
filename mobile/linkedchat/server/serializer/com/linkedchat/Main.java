package com.linkedchat;

import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.util.Base64;
import com.linkedchat.utils;

public class Main {

    public static void main(String[] args) {
        utils.ExecObj obj = new utils.ExecObj();
        
        //obj.NewPublicKey = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA13MJWSQAZjGOJf0D+Ro1Lb6FS/CUWRPuhCwzEZYi/mxoNYTIi13ytjIgjN6A7jCTyy9Xp8Fr0OiirT2IEGTA1lulbND1EaWdg+ap5xUMSkbT1Uj7Vp5jTVjU1wGctqPNxQua6KQ9NBsUQglia0qHQR8IuKM+oWMMKBa17eHcP/utA8RWCNawyfzSawbMnJ3DscR03zwYIn2aYncuiVMwslpDw5LQDIzJj83t+un6llL418Rbd229N1N3N3gLvBUZ65v3DkxWYMoRENgbn72KtZ8z7tvXJQ9jHRioaG2CVB3tL6cxWOqArUXYW5dz8A6390Sw7qh188Ls9+V1nJAeoQIDAQAB";
        // Serialize object to byte array
        byte[] serializedObject = serializeToByteArray(obj);

        // Encode byte array as Base64 string
        String base64String = Base64.getEncoder().encodeToString(serializedObject);

        System.out.println("Serialized and Base64 encoded object:");
        System.out.println(base64String);

        String test = "test";
        System.out.println(test.equals(obj.Key));
        // Decode Base64 string to byte array
        //byte[] decodedBytes = Base64.getDecoder().decode(base64String);

        // Deserialize byte array to object
        //utils.ResecObj deserializedObj = deserializeFromByteArray(decodedBytes);

        // Print deserialized object fields to verify
        //System.out.println("\nDeserialized object:");
    }

    // Method to serialize object to byte array
    private static byte[] serializeToByteArray(Object obj) {
        try {
            ByteArrayOutputStream byteOut = new ByteArrayOutputStream();
            ObjectOutputStream objOut = new ObjectOutputStream(byteOut);
            objOut.writeObject(obj);
            objOut.close();
            return byteOut.toByteArray();
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }

    // Method to deserialize object from byte array
    private static utils.ResecObj deserializeFromByteArray(byte[] data) {
        try {
            ByteArrayInputStream byteIn = new ByteArrayInputStream(data);
            ObjectInputStream objIn = new ObjectInputStream(byteIn);
            Object obj = objIn.readObject();
            objIn.close();
            return (utils.ResecObj) obj;
        } catch (IOException | ClassNotFoundException e) {
            e.printStackTrace();
            return null;
        }
    }
}
