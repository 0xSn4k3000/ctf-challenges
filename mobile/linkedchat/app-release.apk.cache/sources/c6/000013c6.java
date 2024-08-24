package com.linkedchat;

import android.os.Build;
import java.io.File;
import java.io.IOException;
import java.io.Serializable;
import java.security.KeyFactory;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.NoSuchAlgorithmException;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.SecureRandom;
import java.security.spec.X509EncodedKeySpec;
import java.util.Base64;
import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

/* loaded from: classes.dex */
public class utils {
    public static String AESKeyIVString = null;
    public static String AESKeyString = null;
    public static IvParameterSpec aesIv = null;
    public static SecretKeySpec aesKey = null;
    public static KeyPair keyPair = null;
    public static PrivateKey privateKey = null;
    public static String privateKeyString = null;
    public static PublicKey publicKey = null;
    public static String publicKeyString = null;
    public static boolean sym = false;

    /* loaded from: classes.dex */
    public static class ResecObj implements Serializable {
        public String NewPublicKey;
        public String PublicKey;
        public String newSymKey;

        public void changeKeysWithNewOne() {
            utils.AESKeyString = this.newSymKey.split(":", 2)[0];
            utils.AESKeyIVString = this.newSymKey.split(":", 2)[1];
            utils.InitAES();
        }
    }

    public static void GenerateAesKey() {
        try {
            KeyGenerator keyGenerator = KeyGenerator.getInstance("AES");
            keyGenerator.init(256);
            aesKey = new SecretKeySpec(keyGenerator.generateKey().getEncoded(), "AES");
            byte[] bArr = new byte[16];
            new SecureRandom().nextBytes(bArr);
            aesIv = new IvParameterSpec(bArr);
        } catch (Exception e) {
            System.out.println(e.getMessage());
        }
    }

    public static void InitAES() {
        aesIv = new IvParameterSpec(b64decode(AESKeyIVString));
        aesKey = new SecretKeySpec(b64decode(AESKeyString), "AES");
    }

    public static byte[] b64decode(String str) {
        Base64.Decoder decoder;
        byte[] decode;
        if (Build.VERSION.SDK_INT >= 26) {
            decoder = Base64.getDecoder();
            decode = decoder.decode(str);
            return decode;
        }
        return new byte[0];
    }

    public static String b64encode(byte[] bArr) {
        Base64.Encoder encoder;
        String encodeToString;
        if (Build.VERSION.SDK_INT >= 26) {
            encoder = Base64.getEncoder();
            encodeToString = encoder.encodeToString(bArr);
            return encodeToString;
        }
        return new String("");
    }

    public static PublicKey convertFromBase64(String str) throws Exception {
        return KeyFactory.getInstance("RSA").generatePublic(new X509EncodedKeySpec(b64decode(str)));
    }

    public static String decrypt(String str, SecretKeySpec secretKeySpec, IvParameterSpec ivParameterSpec) {
        try {
            Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5PADDING");
            cipher.init(2, secretKeySpec, ivParameterSpec);
            return new String(cipher.doFinal(b64decode(str)));
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    public static String encrypt(String str, SecretKeySpec secretKeySpec, IvParameterSpec ivParameterSpec) {
        try {
            Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5PADDING");
            cipher.init(1, secretKeySpec, ivParameterSpec);
            return new String(b64encode(cipher.doFinal(str.getBytes())));
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    public static KeyPair generateRsaKeys() {
        try {
            KeyPairGenerator keyPairGenerator = KeyPairGenerator.getInstance("RSA");
            keyPairGenerator.initialize(2048);
            KeyPair generateKeyPair = keyPairGenerator.generateKeyPair();
            keyPair = generateKeyPair;
            publicKey = generateKeyPair.getPublic();
            PrivateKey privateKey2 = keyPair.getPrivate();
            privateKey = privateKey2;
            privateKeyString = b64encode(privateKey2.getEncoded());
            publicKeyString = b64encode(publicKey.getEncoded());
            return null;
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
            return null;
        }
    }

    public static boolean isDeviceRooted() {
        File[] fileArr = {new File("/system/app/Superuser.apk"), new File("/sbin/su"), new File("/system/bin/su"), new File("/system/xbin/su"), new File("/data/local/xbin/su"), new File("/data/local/bin/su"), new File("/system/sd/xbin/su"), new File("/system/bin/failsafe/su"), new File("/data/local/su")};
        for (int i = 0; i < 9; i++) {
            if (fileArr[i].exists()) {
                return true;
            }
        }
        try {
            Runtime.getRuntime().exec("su").destroy();
            return true;
        } catch (IOException unused) {
            return false;
        }
    }

    public static String rsaDecrypt(String str, PrivateKey privateKey2) throws Exception {
        byte[] b64decode = b64decode(str);
        Cipher cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
        cipher.init(2, privateKey2);
        return new String(cipher.doFinal(b64decode));
    }

    public static String rsaEncrypt(String str, PublicKey publicKey2) throws Exception {
        Cipher cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
        cipher.init(1, publicKey2);
        return b64encode(cipher.doFinal(str.getBytes()));
    }
}