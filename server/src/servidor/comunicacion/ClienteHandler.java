package servidor.comunicacion;

import java.io.*;
import java.net.Socket;
import servidor.logica.EstadoJuego;
import servidor.logica.ObservadorEstado;
import servidor.logica.modelo.Jugador;

/**
 * COMUNICACION — Maneja la conexión de un cliente individual.
 * Implementa ObservadorEstado (patrón Observer) para recibir
 * actualizaciones del estado del juego automáticamente.
 *
 * Flujo jugador:  conecta -> BIENVENIDO id -> recibe comandos
 * Flujo espectador: conecta -> recibe PARTIDAS -> envía VER id -> observa
 */
public class ClienteHandler implements Runnable, ObservadorEstado {

    private final Socket    socket;
    private       EstadoJuego estado;      // null para espectadores hasta elegir partida
    private       Jugador   jugador;
    private       PrintWriter  salida;
    private       BufferedReader entrada;
    private final boolean    esEspectador;
    private       boolean    conectado = true;
    private       int        partidaId = -1;
    private final Servidor   servidor;
    private final String     tipoControl; // "TECLADO", "PICO" o null para espectadores

    public ClienteHandler(Socket socket, EstadoJuego estado,
                          boolean esEspectador, Servidor servidor,
                          String tipoControl) {
        this.socket       = socket;
        this.estado       = estado;
        this.esEspectador = esEspectador;
        this.servidor     = servidor;
        this.tipoControl  = tipoControl;
    }

    public void setPartidaId(int id) { this.partidaId = id; }

    @Override
    public void run() {
        try {
            salida  = new PrintWriter(socket.getOutputStream(), true);
            entrada = new BufferedReader(new InputStreamReader(socket.getInputStream()));

            if (!esEspectador) {
                runJugador();
            } else {
                runEspectador();
            }

        } catch (IOException e) {
            System.out.println("[SERVIDOR] Cliente desconectado.");
        } finally {
            if (estado != null) {
                if (!esEspectador && partidaId >= 0) {
                    estado.cerrar();
                    servidor.eliminarPartida(partidaId);
                    servidor.liberarSlot(tipoControl);
                }
                estado.eliminarObservador(this);
            }
            cerrar();
        }
    }

    // ── FLUJO JUGADOR ───────────────────────────────────
    private void runJugador() throws IOException {
        estado.agregarObservador(this);
        jugador = estado.agregarJugador();
        salida.println("BIENVENIDO " + jugador.getId());
        System.out.println("[SERVIDOR] Jugador " + jugador.getId() + " en partida " + partidaId);

        String mensaje;
        while (conectado && (mensaje = entrada.readLine()) != null) {
            procesarMensaje(mensaje.trim());
        }
    }

    // ── FLUJO ESPECTADOR ────────────────────────────────
    private void runEspectador() throws IOException {
        // Enviar lista de partidas disponibles
        int numPartidas = servidor.enviarListaPartidas(salida);

        if (numPartidas == 0) {
            System.out.println("[SERVIDOR] Espectador sin partidas disponibles.");
            return;
        }

        // Esperar comando VER <id> del cliente (30 s de timeout)
        socket.setSoTimeout(30000);
        String ver = entrada.readLine();
        socket.setSoTimeout(0);

        if (ver == null || !ver.toUpperCase().startsWith("VER ")) return;

        int elegida;
        try { elegida = Integer.parseInt(ver.substring(4).trim()); }
        catch (NumberFormatException e) { return; }

        estado = servidor.getPartida(elegida);
        if (estado == null) {
            salida.println("PARTIDA_NO_ENCONTRADA");
            return;
        }

        estado.agregarObservador(this);
        salida.println("ESPECTADOR");
        System.out.println("[SERVIDOR] Espectador observando partida " + elegida);

        // Mantener conexión activa; las actualizaciones llegan por actualizar()
        String linea;
        while (conectado && (linea = entrada.readLine()) != null) {
            // espectadores no envían comandos de juego
        }
    }

    // ── MENSAJES DEL JUGADOR ────────────────────────────
    private void procesarMensaje(String mensaje) {
        switch (mensaje.toUpperCase()) {
            case "MOVER_IZQ": estado.moverIzquierda(jugador.getId()); break;
            case "MOVER_DER": estado.moverDerecha(jugador.getId());   break;
            case "DISPARAR":  estado.disparar(jugador.getId());        break;
            default: System.out.println("[SERVIDOR] Comando desconocido: " + mensaje);
        }
    }

    // ── PATRÓN OBSERVER ─────────────────────────────────
    @Override
    public void actualizar(String estadoSerializado) {
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

    public boolean isConectado()  { return conectado;    }
    public boolean isEspectador() { return esEspectador; }
}
