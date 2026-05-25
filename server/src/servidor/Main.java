package servidor;

import servidor.comunicacion.Servidor;

/**
 * CONTROL — Punto de entrada del servidor spaCEinvaders.
 */
public class Main {
    public static void main(String[] args) {
        try {
            new Servidor().iniciar();
        } catch (Exception e) {
            System.err.println("Error fatal: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
