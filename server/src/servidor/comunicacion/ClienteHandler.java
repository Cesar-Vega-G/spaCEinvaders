package servidor.comunicacion;

import java.io.*;
import java.net.Socket;
import servidor.logica.EstadoJuego;
import servidor.logica.ObservadorEstado;
import servidor.logica.modelo.Jugador;

/**
 * COMUNICACION — Maneja la conexión de un cliente individual (hilo propio).
 *
 * Implementa el patrón Observer: se registra en EstadoJuego y, cada vez que
 * el game loop serializa el estado, este handler lo reenvía al cliente TCP.
 *
 * Dos flujos según el rol del cliente:
 *   · Jugador   → recibe "BIENVENIDO id", envía comandos (MOVER_IZQ / DISPARAR…)
 *   · Espectador → recibe lista de partidas, elige con "VER id", solo lee
 */
public class ClienteHandler implements Runnable, ObservadorEstado {

    private final Socket      socket;
    private       EstadoJuego estado;       /* null hasta que el espectador elige partida */
    private       Jugador     jugador;
    private       PrintWriter salida;
    private       BufferedReader entrada;
    private final boolean     esEspectador;
    private       boolean     conectado  = true;
    private       int         partidaId  = -1;
    private final Servidor    servidor;
    private final String      tipoControl; /* "TECLADO", "PICO" o null (espectadores) */

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

            if (!esEspectador) runJugador();
            else               runEspectador();

        } catch (IOException e) {
            System.out.println("[SERVIDOR] Cliente desconectado.");
        } finally {
            /*
             * Al desconectarse el jugador: cerrar la partida, eliminarla del
             * mapa y liberar el slot para que otro jugador pueda conectarse.
             */
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

    /* ── FLUJO JUGADOR ─────────────────────────────────────────────────── */
    private void runJugador() throws IOException {
        estado.agregarObservador(this);
        jugador = estado.agregarJugador();
        /* Confirmar la conexión enviando el id asignado. */
        salida.println("BIENVENIDO " + jugador.getId());
        System.out.println("[SERVIDOR] Jugador " + jugador.getId() + " en partida " + partidaId);

        String mensaje;
        while (conectado && (mensaje = entrada.readLine()) != null) {
            procesarMensaje(mensaje.trim());
        }
    }

    /* ── FLUJO ESPECTADOR ──────────────────────────────────────────────── */
    private void runEspectador() throws IOException {
        int numPartidas = servidor.enviarListaPartidas(salida);

        if (numPartidas == 0) {
            System.out.println("[SERVIDOR] Espectador sin partidas disponibles.");
            return;
        }

        /* Esperar que el espectador elija partida con "VER id" (timeout 30 s). */
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

        /* Mantener hilo vivo; las actualizaciones llegan via actualizar(). */
        String linea;
        while (conectado && (linea = entrada.readLine()) != null) {
            /* Los espectadores no envían comandos. */
        }
    }

    /* ── PROCESAR COMANDOS DEL JUGADOR ────────────────────────────────── */
    private void procesarMensaje(String mensaje) {
        switch (mensaje.toUpperCase()) {
            case "MOVER_IZQ": estado.moverIzquierda(jugador.getId()); break;
            case "MOVER_DER": estado.moverDerecha(jugador.getId());   break;
            case "DISPARAR":  estado.disparar(jugador.getId());        break;
            default: System.out.println("[SERVIDOR] Comando desconocido: " + mensaje);
        }
    }

    /* ── PATRÓN OBSERVER ───────────────────────────────────────────────── */
    /**
     * Llamado por EstadoJuego.notificarObservadores() fuera del lock del
     * game loop. Reenvía el estado serializado al cliente por el socket.
     */
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
