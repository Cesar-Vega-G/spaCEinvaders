package servidor.comunicacion;

import servidor.logica.EstadoJuego;
import servidor.logica.modelo.Jugador;

import java.io.*;
import java.net.Socket;

/**
 * COMUNICACION — Maneja la conexión de un cliente individual.
 * Un hilo por cliente (jugador o espectador).
 */
public class ClienteHandler implements Runnable {

    private Socket socket;
    private EstadoJuego estado;
    private Jugador jugador;
    private PrintWriter salida;
    private BufferedReader entrada;
    private boolean esEspectador;
    private boolean conectado;

    public ClienteHandler(Socket socket, EstadoJuego estado, boolean esEspectador) {
        this.socket      = socket;
        this.estado      = estado;
        this.esEspectador = esEspectador;
        this.conectado   = true;
    }

    @Override
    public void run() {
        try {
            salida  = new PrintWriter(socket.getOutputStream(), true);
            entrada = new BufferedReader(new InputStreamReader(socket.getInputStream()));

            if (!esEspectador) {
                jugador = estado.agregarJugador();
                salida.println("BIENVENIDO " + jugador.getId());
                System.out.println("[SERVIDOR] Jugador " + jugador.getId() + " conectado.");
            } else {
                salida.println("ESPECTADOR");
                System.out.println("[SERVIDOR] Espectador conectado.");
            }

            String mensaje;
            while (conectado && (mensaje = entrada.readLine()) != null) {
                if (!esEspectador) procesarMensaje(mensaje.trim());
            }

        } catch (IOException e) {
            System.out.println("[SERVIDOR] Cliente desconectado.");
        } finally {
            cerrar();
        }
    }

    private void procesarMensaje(String mensaje) {
        switch (mensaje.toUpperCase()) {
            case "MOVER_IZQ": estado.moverIzquierda(jugador.getId()); break;
            case "MOVER_DER": estado.moverDerecha(jugador.getId());   break;
            case "DISPARAR":  estado.disparar(jugador.getId());        break;
            default: System.out.println("[SERVIDOR] Comando desconocido: " + mensaje);
        }
    }

    public void enviarEstado(String estadoSerializado) {
        if (salida != null && conectado) {
            salida.print(estadoSerializado);
            salida.flush();
        }
    }

    private void cerrar() {
        conectado = false;
        try { if (socket != null) socket.close(); }
        catch (IOException e) { /* ignorar */ }
    }

    public boolean isConectado() { return conectado; }
}
