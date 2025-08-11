package com.stehno.mermaid;

public class MermaidException extends Exception {
    public MermaidException() {
        super();
    }

    public MermaidException(String message) {
        super(message);
    }

    public MermaidException(String message, Throwable cause) {
        super(message, cause);
    }

    public MermaidException(Throwable cause) {
        super(cause);
    }
}