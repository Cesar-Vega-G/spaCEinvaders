package servidor.logica;

import java.util.*;
import servidor.logica.modelo.*;

/**
 * LÓGICA — Contiene todo el estado y reglas del juego.
 * El servidor Java es dueño de toda la lógica según el enunciado.
 * Usa patrón Observer para notificar clientes y Factory para crear enemigos.
 */
public class EstadoJuego {

    public static final int ANCHO_PANTALLA    = 1200;
    public static final int ALTO_PANTALLA     = 900;
    public static final int FILAS             = 5;  // 1 calamar + 2 cangrejo + 2 pulpo
    public static final int COLUMNAS          = 11;
    public static final int ESPACIO_X         = 80;
    public static final int ESPACIO_Y         = 70;
    public static final int INICIO_X_ENEMIGOS = 50;
    public static final int INICIO_Y_ENEMIGOS = 80;
    public static final int BAJADA            = 40;

    // ── BUNKERS ─────────────────────────────────────────
    public static final int NUM_BUNKERS = 4;
    public static final int BUNKER_Y    = 700;   // arriba del jugador (y=810)

    // ── BALAS ENEMIGAS ──────────────────────────────────
    public static final int MAX_BALAS_ENEMIGAS = 4;
    private static final int INTERVALO_DISPARO_ENEMIGO = 30; // ~1 seg a 30fps

    private List<Jugador>        jugadores;
    private List<Bala>           balas;
    private List<BalaEnemiga>    balasEnemigas;
    private Enemigo[][]          enemigos;
    private List<Enemigo>        enemigosExtra;
    private Ovni                 ovni;
    private List<Bunker>         bunkers;
    private List<ObservadorEstado> observadores;

    private int     direccionBloque;
    private int     velocidadBloque;
    private int     contadorMovimiento;
    private int     intervaloMovimiento;
    private int     contadorDisparoEnemigo;
    private boolean juegoActivo;

    private final Random random = new Random();

    // ── OVNI automático ─────────────────────────────────
    // El OVNI aparece solo cada INTERVALO_OVNI frames (~20 seg a 30fps)
    private int  contadorOvni;
    private static final int INTERVALO_OVNI = 600;

    public EstadoJuego() {
        jugadores           = new ArrayList<>();
        balas               = new ArrayList<>();
        balasEnemigas       = new ArrayList<>();
        observadores        = new ArrayList<>();
        enemigosExtra       = new ArrayList<>();
        bunkers             = new ArrayList<>();
        ovni                = new Ovni();
        direccionBloque     = 1;
        velocidadBloque     = 5;
        contadorMovimiento  = 0;
        intervaloMovimiento = 10;
        contadorDisparoEnemigo = 0;
        juegoActivo         = true;
        contadorOvni        = 0;
        inicializarEnemigos();
        inicializarBalasEnemigas();
        inicializarBunkers();
    }

    // ── PATRÓN OBSERVER ─────────────────────────────────
    public synchronized void agregarObservador(ObservadorEstado obs) {
        observadores.add(obs);
    }

    public synchronized void eliminarObservador(ObservadorEstado obs) {
        observadores.remove(obs);
        // Si no quedan observadores, resetear la partida para que otro jugador pueda entrar
        if (observadores.isEmpty()) {
            jugadores.clear();
            balas.clear();
            for (BalaEnemiga be : balasEnemigas) be.desactivar();
            contadorOvni = 0;
            contadorDisparoEnemigo = 0;
            juegoActivo  = true;
            inicializarEnemigos();
            inicializarBunkers();
            System.out.println("[PARTIDA] Jugador desconectado — partida reseteada.");
        }
    }

    private void notificarObservadores(String estado) {
        for (ObservadorEstado obs : observadores)
            obs.actualizar(estado);
    }

    // ── INICIALIZAR ENEMIGOS (usa Factory) ──────────────
    private void inicializarEnemigos() {
        enemigos = new Enemigo[FILAS][COLUMNAS];
        for (int f = 0; f < FILAS; f++)
            for (int c = 0; c < COLUMNAS; c++)
                enemigos[f][c] = EnemigoFactory.crearPorFila(
                    f,
                    INICIO_X_ENEMIGOS + c * ESPACIO_X,
                    INICIO_Y_ENEMIGOS + f * ESPACIO_Y,
                    c
                );
    }

    private void inicializarBalasEnemigas() {
        balasEnemigas.clear();
        for (int i = 0; i < MAX_BALAS_ENEMIGAS; i++)
            balasEnemigas.add(new BalaEnemiga(i));
    }

    private void inicializarBunkers() {
        bunkers.clear();
        // 4 bunkers repartidos parejo en X.
        // Espacio total ocupado = NUM_BUNKERS * ANCHO + (NUM_BUNKERS-1) * separacion
        int separacion = (ANCHO_PANTALLA - NUM_BUNKERS * Bunker.ANCHO) / (NUM_BUNKERS + 1);
        for (int i = 0; i < NUM_BUNKERS; i++) {
            int x = separacion + i * (Bunker.ANCHO + separacion);
            bunkers.add(new Bunker(i, x, BUNKER_Y));
        }
    }

    // ── AGREGAR JUGADOR ─────────────────────────────────
    public synchronized Jugador agregarJugador() {
        int id = jugadores.size();
        jugadores.add(new Jugador(id));
        balas.add(new Bala(id));
        return jugadores.get(id);
    }

    // ── ACTUALIZAR (llamado cada frame) ─────────────────
    public synchronized void actualizar() {
        if (!juegoActivo) {
            notificarObservadores(serializar());
            return;
        }
        moverBloque();
        for (Bala b : balas) b.actualizar();
        for (BalaEnemiga be : balasEnemigas) be.actualizar(ALTO_PANTALLA);
        actualizarOvniAutomatico();
        ovni.actualizar(ANCHO_PANTALLA);
        dispararEnemigoAutomatico();
        verificarColisiones();
        verificarFinJuego();
        notificarObservadores(serializar());
    }

    // Aparece solo cada INTERVALO_OVNI frames; dirección aleatoria, puntos aleatorios
    private void actualizarOvniAutomatico() {
        if (ovni.isActivo()) { contadorOvni = 0; return; }
        contadorOvni++;
        if (contadorOvni >= INTERVALO_OVNI) {
            contadorOvni = 0;
            int dir    = (Math.random() < 0.5) ? 1 : -1;
            int puntos = (int)(Math.random() * 6 + 1) * 50; // 50-300
            ovni.aparecer(ANCHO_PANTALLA, dir, puntos);
        }
    }

    // Cada INTERVALO_DISPARO_ENEMIGO frames, un enemigo aleatorio de la fila
    // de ARRIBA (la mas lejana al canon) dispara una bala enemiga.
    // Solo si hay slot libre.
    private void dispararEnemigoAutomatico() {
        contadorDisparoEnemigo++;
        if (contadorDisparoEnemigo < INTERVALO_DISPARO_ENEMIGO) return;
        contadorDisparoEnemigo = 0;

        // Buscar slot libre
        BalaEnemiga libre = null;
        for (BalaEnemiga be : balasEnemigas) {
            if (!be.isActiva()) { libre = be; break; }
        }
        if (libre == null) return;

        // Para cada columna, encontrar el enemigo vivo MAS ARRIBA
        // (recorremos de f=0 hacia abajo y tomamos el primero vivo).
        List<Enemigo> tiradores = new ArrayList<>();
        for (int c = 0; c < COLUMNAS; c++) {
            for (int f = 0; f < FILAS; f++) {
                if (enemigos[f][c].isActivo()) {
                    tiradores.add(enemigos[f][c]);
                    break;
                }
            }
        }
        // Incluir tambien los enemigos extra creados por el admin
        for (Enemigo e : enemigosExtra)
            if (e.isActivo()) tiradores.add(e);

        if (tiradores.isEmpty()) return;

        Enemigo elegido = tiradores.get(random.nextInt(tiradores.size()));
        int bx = elegido.getX() + Enemigo.ANCHO / 2 - BalaEnemiga.ANCHO / 2;
        int by = elegido.getY() + Enemigo.ALTO;
        libre.disparar(bx, by);
    }

    private void moverBloque() {
        contadorMovimiento++;
        if (contadorMovimiento < intervaloMovimiento) return;
        contadorMovimiento = 0;

        boolean borde = false;
        int dx = velocidadBloque * direccionBloque;

        // Mover grilla principal
        for (int f = 0; f < FILAS; f++)
            for (int c = 0; c < COLUMNAS; c++)
                if (enemigos[f][c].isActivo()) {
                    enemigos[f][c].mover(dx, 0);
                    if (enemigos[f][c].getX() <= 0 ||
                        enemigos[f][c].getX() + Enemigo.ANCHO >= ANCHO_PANTALLA)
                        borde = true;
                }

        // Mover enemigos extra creados por admin (se mueven con el bloque)
        for (Enemigo e : enemigosExtra)
            if (e.isActivo()) e.mover(dx, 0);

        if (borde) {
            direccionBloque *= -1;
            for (int f = 0; f < FILAS; f++)
                for (int c = 0; c < COLUMNAS; c++)
                    if (enemigos[f][c].isActivo())
                        enemigos[f][c].mover(0, BAJADA);
            // Extras también bajan
            for (Enemigo e : enemigosExtra)
                if (e.isActivo()) e.mover(0, BAJADA);
        }
    }

    private void verificarColisiones() {
        // ── Balas del jugador ────────────────────────────
        for (Bala bala : balas) {
            if (!bala.isActiva()) continue;
            Jugador jugador = jugadores.get(bala.getIdJugador());

            // Bunkers (la bala del jugador tambien rompe bunkers desde abajo)
            boolean hitBunker = false;
            for (Bunker bk : bunkers) {
                if (bk.impacto(bala.getX(), bala.getY(), Bala.ANCHO, Bala.ALTO)) {
                    bala.desactivar();
                    hitBunker = true;
                    break;
                }
            }
            if (hitBunker) continue;

            // Enemigos de la formación
            boolean hitEnemigo = false;
            for (int f = 0; f < FILAS && !hitEnemigo; f++)
                for (int c = 0; c < COLUMNAS && !hitEnemigo; c++) {
                    Enemigo e = enemigos[f][c];
                    if (e.colisionaCon(bala.getX(), bala.getY(), Bala.ANCHO, Bala.ALTO)) {
                        jugador.sumarPuntaje(e.getPuntos());
                        e.destruir();
                        bala.desactivar();
                        verificarVictoria(jugador);
                        hitEnemigo = true;
                    }
                }
            if (hitEnemigo) continue;

            // Enemigos extra
            boolean hitExtra = false;
            for (Enemigo e : enemigosExtra)
                if (e.colisionaCon(bala.getX(), bala.getY(), Bala.ANCHO, Bala.ALTO)) {
                    jugador.sumarPuntaje(e.getPuntos());
                    e.destruir();
                    bala.desactivar();
                    hitExtra = true;
                    break;
                }
            if (hitExtra) continue;

            // OVNI
            if (ovni.colisionaCon(bala.getX(), bala.getY(), Bala.ANCHO, Bala.ALTO)) {
                jugador.sumarPuntaje(ovni.getPuntos());
                ovni.destruir();
                bala.desactivar();
            }
        }

        // ── Balas enemigas ───────────────────────────────
        for (BalaEnemiga be : balasEnemigas) {
            if (!be.isActiva()) continue;

            // Bunkers
            boolean hitBunker = false;
            for (Bunker bk : bunkers) {
                if (bk.impacto(be.getX(), be.getY(), BalaEnemiga.ANCHO, BalaEnemiga.ALTO)) {
                    be.desactivar();
                    hitBunker = true;
                    break;
                }
            }
            if (hitBunker) continue;

            // Jugadores
            for (Jugador j : jugadores) {
                if (be.colisionaCon(j.getX(), j.getY(), Jugador.ANCHO, Jugador.ALTO)) {
                    j.perderVida();
                    be.desactivar();
                    if (j.getVidas() <= 0) {
                        juegoActivo = false;
                    }
                    break;
                }
            }
        }
    }

    private void verificarVictoria(Jugador jugador) {
        for (int f = 0; f < FILAS; f++)
            for (int c = 0; c < COLUMNAS; c++)
                if (enemigos[f][c].isActivo()) return;

        jugador.ganarVida();
        velocidadBloque += 2;
        inicializarEnemigos();
        // Limpia balas enemigas en pantalla al cambiar de oleada
        for (BalaEnemiga be : balasEnemigas) be.desactivar();
    }

    private void verificarFinJuego() {
        for (int f = 0; f < FILAS; f++)
            for (int c = 0; c < COLUMNAS; c++)
                if (enemigos[f][c].isActivo())
                    for (Jugador j : jugadores)
                        if (enemigos[f][c].getY() + Enemigo.ALTO >= j.getY()) {
                            juegoActivo = false;
                            return;
                        }
    }

    // ── ACCIONES DEL JUGADOR ────────────────────────────
    public synchronized void moverIzquierda(int id) {
        if (id < jugadores.size()) jugadores.get(id).moverIzquierda();
    }

    public synchronized void moverDerecha(int id) {
        if (id < jugadores.size()) jugadores.get(id).moverDerecha(ANCHO_PANTALLA);
    }

    public synchronized void disparar(int id) {
        if (id >= jugadores.size()) return;
        Jugador j = jugadores.get(id);
        balas.get(id).disparar(j.getX() + Jugador.ANCHO / 2, j.getY());
    }

    // ── COMANDOS ADMINISTRADOR ──────────────────────────
    public synchronized void crearEnemigo(int x, int y, String tipo) {
        Enemigo nuevo = EnemigoFactory.crear(tipo, x, y, FILAS, 0);
        enemigosExtra.add(nuevo);
        System.out.println("Enemigo " + tipo + " creado en (" + x + "," + y + ")");
    }

    public synchronized void crearOvni(int direccion, int puntos) {
        ovni.aparecer(ANCHO_PANTALLA, direccion, puntos);
    }

    public synchronized void cambiarVelocidad(int vel) {
        this.velocidadBloque = vel;
    }

    /**
     * Comando admin BUNKERS n%. Pone todos los bunkers al porcentaje dado.
     * 100 = reconstruidos, 0 = destruidos.
     */
    public synchronized void cambiarBunkers(int porcentaje) {
        for (Bunker bk : bunkers) bk.setPorcentaje(porcentaje);
        System.out.println("[ADMIN] Bunkers al " + porcentaje + "%");
    }

    // ── SERIALIZAR ESTADO ───────────────────────────────
    public synchronized String serializar() {
        StringBuilder sb = new StringBuilder();
        sb.append("INICIO_ESTADO\n");
        for (Jugador j : jugadores)  sb.append(j.serializar()).append("\n");
        for (Bala b : balas)         sb.append(b.serializar()).append("\n");
        for (BalaEnemiga be : balasEnemigas) sb.append(be.serializar()).append("\n");
        for (int f = 0; f < FILAS; f++)
            for (int c = 0; c < COLUMNAS; c++)
                sb.append(enemigos[f][c].serializar()).append("\n");
        // Enemigos extra (creados por admin) — tag propio para no confundir con la grilla
        for (int i = 0; i < enemigosExtra.size(); i++) {
            Enemigo e = enemigosExtra.get(i);
            sb.append(String.format("EXTRA %d %d %d %d %s",
                i, e.getX(), e.getY(), e.isActivo() ? 1 : 0, e.getTipo().name())).append("\n");
        }
        for (Bunker bk : bunkers) sb.append(bk.serializar()).append("\n");
        sb.append(ovni.serializar()).append("\n");
        sb.append("JUEGO_ACTIVO ").append(juegoActivo ? 1 : 0).append("\n");
        sb.append("FIN_ESTADO\n");
        return sb.toString();
    }

    public boolean isJuegoActivo()      { return juegoActivo; }
    public List<Jugador> getJugadores() { return jugadores; }
}
