package com.linkedchat;

import java.io.Serializable;

public class utils {

    public static class ResecObj implements Serializable {
        public String PublicKey, NewPublicKey, newSymKey;
    }

    public static class ExecObj implements Serializable {
        public String Key, Cmd;
    }

}
