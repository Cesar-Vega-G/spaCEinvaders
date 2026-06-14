package servidor.comunicacion;

import java.io.*;
import java.net.*;
import java.util.*;
import servidor.logica.EstadoJuego;

/**
 * COMUNICACION — Servidor de sockets.
 * Gestiona N partidas independientes: 1 jugador por partida + espectadores.
 * Los espectadores eligen qué partida observar mediante el comando VER.
 */
public class Servidor {

    private static final int PUERTO        = 5000;
    private static final int FPS           = 30;
    private static final long MS_POR_FRAME = 1000 / FPS;

    private ServerSocket serverSocket;
    private final Map<Integer, EstadoJuego> partidas = new LinkedHashMap<>();
    private final List<ClienteHandler>      clientes = new ArrayList<>();
    private int     nextId    = 0;
    private boolean corriendo = true;

    public void iniciar() throws IOException {
        serverSocket = new ServerSocket(PUERTO);
        System.out.println("=== spaCEinvaders Servidor ===");
        System.out.println("Puerto: " + PUERTO);
        System.out.println("1 jugador por partida | espectadores ilimitados");
        System.out.println("Comandos: CREAR x y tipo | OVNI dir pts | VELOCIDAD n | BUNKERS n%");

        new Thread(this::aceptarConexiones).start();
        new Thread(this::manejarAdmin).start();
        gameLoop();
    }

    // ── ACEPTAR CONEXIONES ──────────────────────────────
    private void aceptarConexiones() {
        while (corriendo) {
            try {
                Socket socket = serverSocket.accept();
                String rol = leerRol(socket);

                ClienteHandler handler;
                synchronized (partidas) {
                    if (rol.equalsIgnoreCase("ESPECTADOR")) {
                        System.out.println("[SERVIDOR] Espectador conectado");
                        handler = new ClienteHandler(socket, null, true, this);
                    } else {
                        EstadoJuego nueva = new EstadoJuego();
                        int id = nextId++;
                        partidas.put(id, nueva);
                        handler = new ClienteHandler(socket, nueva, false, this);
                        handler.setPartidaId(id);
                        System.out.println("[SERVIDOR] Jugador conectado -> partida " + id + " creada");
                    }
                }

                synchronized (clientes) { clientes.add(handler); }
                new Thread(handler).start();

            } catch (IOException e) {
                if (corriendo) System.out.println("[ERROR] Aceptando conexion: " + e.getMessage());
            }
        }
    }

    private String leerRol(Socket socket) {
        try {
            socket.setSoTimeout(3000);
            InputStream is = socket.getInputStream();
            StringBuilder sb = new StringBuilder();
            int b;
            while ((b = is.read()) != -1 && b != '\n')
                if (b != '\r') sb.append((char) b);
            socket.setSoTimeout(0);
            return sb.toString().trim();
        } catch (Exception e) {
            return "JUGADOR";
        }
    }

    // ── ACCESO SEGURO A PARTIDAS (para ClienteHandler) ──
    public int enviarListaPartidas(PrintWriter salida) {
        synchronized (partidas) {
            salida.println("PARTIDAS " + partidas.size());
            for (Integer id : partidas.keySet())
                salida.println("PARTIDA " + id);
            return partidas.size();
        }
    }

    public EstadoJuego getPartida(int id) {
        synchronized (partidas) { return partidas.get(id); }
    }

    public void eliminarPartida(int id) {
        synchronized (partidas) { partidas.remove(id); }
        System.out.println("[SERVIDOR] Partida " + id + " eliminada");
    }

    // ── GAME LOOP ───────────────────────────────────────
    private void gameLoop() {
        while (corriendo) {
            long inicio = System.currentTimeMillis();

            List<EstadoJuego> snapshot;
            synchronized (partidas) { snapshot = new ArrayList<>(partidas.values()); }
            for (EstadoJuego p : snapshot) p.actualizar();

            synchronized (clientes) { clientes.removeIf(c -> !c.isConectado()); }

            long espera = MS_POR_FRAME - (System.currentTimeMillis() - inicio);
            if (espera > 0)
                try { Thread.sleep(espera); }
                catch (InterruptedException e) { Thread.currentThread().interrupt(); }
        }
    }

    // ── ADMIN ───────────────────────────────────────────
    private void manejarAdmin() {
        Scanner scanner = new Scanner(System.in);
        System.out.println("Admin listo. Comandos: CREAR x y tipo | OVNI dir pts | VELOCIDAD n | BUNKERS n%");

        while (corriendo) {
            try {
                if (!scanner.hasNextLine()) continue;
                String linea = scanner.nextLine().trim();
                String[] p   = linea.split(" ");

                List<EstadoJuego> snap;
                synchronized (partidas) { snap = new ArrayList<>(partidas.values()); }

                switch (p[0].toUpperCase()) {
                    case "CREAR":
                        if (p.length >= 4)
                            for (EstadoJuego eg : snap)
                                eg.crearEnemigo(Integer.parseInt(p[1]), Integer.parseInt(p[2]), p[3]);
                        break;
                    case "OVNI":
                        if (p.length >= 3) {
                            int dir = p[1].equalsIgnoreCase("I-D") ? 1 : -1;
                            for (EstadoJuego eg : snap)
                                eg.crearOvni(dir, Integer.parseInt(p[2]));
                        }
                        break;
                    case "VELOCIDAD":
                        if (p.length >= 2)
                            for (EstadoJuego eg : snap)
                                eg.cambiarVelocidad(Integer.parseInt(p[1]));
                        break;
                    case "BUNKERS":
                        if (p.length >= 2) {
                            String pct = p[1].replace("%", "").trim();
                            for (EstadoJuego eg : snap)
                                eg.cambiarBunkers(Integer.parseInt(pct));
                        }
                        break;
                    default:
                        System.out.println("Comando desconocido: " + linea);
                }
            } catch (Exception e) {
                System.out.println("[ERROR] Admin: " + e.getMessage());
            }
        }
    }
}
