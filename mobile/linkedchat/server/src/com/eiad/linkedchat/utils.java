package com.eiad.linkedchat;

import java.io.Serializable;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.KeyFactory;
import java.security.spec.X509EncodedKeySpec;
import java.util.Arrays;
import java.util.Base64;

import javax.crypto.Cipher;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.SecretKey;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.NoSuchAlgorithmException;

import java.io.*;

public class f0 {

    public static class f implements Serializable {
        public String PublicKey, NewPublicKey, newSymKey;
        
        public void changeKeysWithNewOne() {
            System.out.println("[+] hacked the system");
        }

        private final void readObject(ObjectInputStream in) throws IOException, ClassNotFoundException {
            in.defaultReadObject();
            System.out.println("[+] Read Object Response!");
        }

    }

}
