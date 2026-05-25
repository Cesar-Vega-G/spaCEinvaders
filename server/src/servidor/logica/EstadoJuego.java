package servidor.logica;

import servidor.logica.modelo.*;
import java.util.*;

/**
 * LÓGICA — Contiene todo el estado y reglas del juego.
 * El servidor Java es dueño de toda la lógica según el enunciado.
 */
public class EstadoJuego {

    public static final int ANCHO_PANTALLA    = 1200;
    public static final int ALTO_PANTALLA     = 900;
    public static final int FILAS             = 3;
    public static final int COLUMNAS          = 11;
    public static final int ESPACIO_X         = 80;
    public static final int ESPACIO_Y         = 70;
    public static final int INICIO_X_ENEMIGOS = 50;
    public static final int INICIO_Y_ENEMIGOS = 80;
    public static final int BAJADA            = 40;

    private List<Jugador> jugadores;
    private List<Bala>    balas;
    private Enemigo[][]   enemigos;
    private Ovni          ovni;

    private int  direccionBloque;
    private int  velocidadBloque;
    private int  contadorMovimiento;
    private int  intervaloMovimiento;
    private boolean juegoActivo;

    public EstadoJuego() {
        jugadores            = new ArrayList<>();
        balas                = new ArrayList<>();
        ovni                 = new Ovni();
        direccionBloque      = 1;
        velocidadBloque      = 5;
        contadorMovimiento   = 0;
        intervaloMovimiento  = 10;
        juegoActivo          = true;
        inicializarEnemigos();
    }

    private void inicializarEnemigos() {
        enemigos = new Enemigo[FILAS][COLUMNAS];
        Enemigo.Tipo[] tipos = {
            Enemigo.Tipo.CALAMAR,
            Enemigo.Tipo.CANGREJO,
            Enemigo.Tipo.PULPO
        };
        for (int f = 0; f < FILAS; f++)
            for (int c = 0; c < COLUMNAS; c++)
                enemigos[f][c] = new Enemigo(
                    INICIO_X_ENEMIGOS + c * ESPACIO_X,
                    INICIO_Y_ENEMIGOS + f * ESPACIO_Y,
                    tipos[f], f, c
                );
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
        if (!juegoActivo) return;
        moverBloque();
        for (Bala b : balas) b.actualizar();
        ovni.actualizar(ANCHO_PANTALLA);
        verificarColisiones();
        verificarFinJuego();
    }

    private void moverBloque() {
        contadorMovimiento++;
        if (contadorMovimiento < intervaloMovimiento) return;
        contadorMovimiento = 0;

        boolean borde = false;
        for (int f = 0; f < FILAS; f++)
            for (int c = 0; c < COLUMNAS; c++)
                if (enemigos[f][c].isActivo()) {
                    enemigos[f][c].mover(velocidadBloque * direccionBloque, 0);
                    if (enemigos[f][c].getX() <= 0 ||
                        enemigos[f][c].getX() + Enemigo.ANCHO >= ANCHO_PANTALLA)
                        borde = true;
                }

        if (borde) {
            direccionBloque *= -1;
            for (int f = 0; f < FILAS; f++)
                for (int c = 0; c < COLUMNAS; c++)
                    if (enemigos[f][c].isActivo())
                        enemigos[f][c].mover(0, BAJADA);
        }
    }

    private void verificarColisiones() {
        for (Bala bala : balas) {
            if (!bala.isActiva()) continue;
            Jugador jugador = jugadores.get(bala.getIdJugador());

            for (int f = 0; f < FILAS; f++)
                for (int c = 0; c < COLUMNAS; c++) {
                    Enemigo e = enemigos[f][c];
                    if (e.colisionaCon(bala.getX(), bala.getY(), Bala.ANCHO, Bala.ALTO)) {
                        jugador.sumarPuntaje(e.getPuntos());
                        e.destruir();
                        bala.desactivar();
                        verificarVictoria(jugador);
                        return;
                    }
                }

            if (ovni.colisionaCon(bala.getX(), bala.getY(), Bala.ANCHO, Bala.ALTO)) {
                jugador.sumarPuntaje(ovni.getPuntos());
                ovni.destruir();
                bala.desactivar();
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
    public synchronized void crearEnemigo(int x, int y, int puntos) {
        System.out.println("Enemigo creado en (" + x + "," + y + ") pts=" + puntos);
    }

    public synchronized void crearOvni(int direccion, int puntos) {
        ovni.aparecer(ANCHO_PANTALLA, direccion, puntos);
    }

    public synchronized void cambiarVelocidad(int vel) {
        this.velocidadBloque = vel;
    }

    // ── SERIALIZAR ESTADO ───────────────────────────────
    public synchronized String serializar() {
        StringBuilder sb = new StringBuilder();
        sb.append("INICIO_ESTADO\n");
        for (Jugador j : jugadores) sb.append(j.serializar()).append("\n");
        for (Bala b : balas)        sb.append(b.serializar()).append("\n");
        for (int f = 0; f < FILAS; f++)
            for (int c = 0; c < COLUMNAS; c++)
                sb.append(enemigos[f][c].serializar()).append("\n");
        sb.append(ovni.serializar()).append("\n");
        sb.append("JUEGO_ACTIVO ").append(juegoActivo ? 1 : 0).append("\n");
        sb.append("FIN_ESTADO\n");
        return sb.toString();
    }

    public boolean isJuegoActivo()      { return juegoActivo; }
    public List<Jugador> getJugadores() { return jugadores; }
}
