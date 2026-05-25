package servidor.comunicacion;

import servidor.logica.EstadoJuego;

import java.io.*;
import java.net.*;
import java.util.*;

/**
 * COMUNICACION — Servidor de sockets.
 * Acepta conexiones, corre el game loop y envía estado a todos los clientes.
 */
public class Servidor {

    private static final int PUERTO        = 5000;
    private static final int MAX_JUGADORES = 2;
    private static final int FPS           = 30;
    private static final long MS_POR_FRAME = 1000 / FPS;

    private ServerSocket serverSocket;
    private EstadoJuego estado;
    private List<ClienteHandler> clientes;
    private boolean corriendo;

    public Servidor() {
        estado    = new EstadoJuego();
        clientes  = new ArrayList<>();
        corriendo = true;
    }

    public void iniciar() throws IOException {
        serverSocket = new ServerSocket(PUERTO);
        System.out.println("=== spaCEinvaders Servidor ===");
        System.out.println("Puerto: " + PUERTO);
        System.out.println("Comandos admin: CREAR x y pts | OVNI I-D pts | VELOCIDAD n");

        // Hilo: aceptar conexiones
        new Thread(this::aceptarConexiones).start();

        // Hilo: comandos del administrador por consola
        new Thread(this::manejarAdmin).start();

        // Game loop en hilo principal
        gameLoop();
    }

    private void aceptarConexiones() {
        while (corriendo) {
            try {
                Socket socket = serverSocket.accept();
                long jugadoresActivos = clientes.stream().filter(ClienteHandler::isConectado).count();
                boolean esEspectador  = jugadoresActivos >= MAX_JUGADORES;

                ClienteHandler handler = new ClienteHandler(socket, estado, esEspectador);
                clientes.add(handler);
                new Thread(handler).start();

            } catch (IOException e) {
                if (corriendo) System.out.println("[ERROR] Aceptando conexión: " + e.getMessage());
            }
        }
    }

    private void gameLoop() {
        while (corriendo) {
            long inicio = System.currentTimeMillis();

            estado.actualizar();
            String estadoStr = estado.serializar();

            clientes.removeIf(c -> !c.isConectado());
            for (ClienteHandler c : clientes) c.enviarEstado(estadoStr);

            long espera = MS_POR_FRAME - (System.currentTimeMillis() - inicio);
            if (espera > 0) {
                try { Thread.sleep(espera); }
                catch (InterruptedException e) { Thread.currentThread().interrupt(); }
            }
        }
    }

    private void manejarAdmin() {
        try (BufferedReader consola = new BufferedReader(new InputStreamReader(System.in))) {
            String linea;
            while ((linea = consola.readLine()) != null) {
                String[] p = linea.trim().split(" ");
                switch (p[0].toUpperCase()) {
                    case "CREAR":
                        if (p.length >= 4)
                            estado.crearEnemigo(Integer.parseInt(p[1]),
                                                Integer.parseInt(p[2]),
                                                Integer.parseInt(p[3]));
                        break;
                    case "OVNI":
                        if (p.length >= 3)
                            estado.crearOvni(p[1].equalsIgnoreCase("I-D") ? 1 : -1,
                                             Integer.parseInt(p[2]));
                        break;
                    case "VELOCIDAD":
                        if (p.length >= 2)
                            estado.cambiarVelocidad(Integer.parseInt(p[1]));
                        break;
                    default:
                        System.out.println("Comando desconocido: " + linea);
                }
            }
        } catch (IOException e) {
            System.out.println("[ERROR] Admin: " + e.getMessage());
        }
    }
}
