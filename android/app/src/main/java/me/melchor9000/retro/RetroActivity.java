package me.melchor9000.retro;

import android.os.Bundle;
import android.util.DisplayMetrics;

import org.libsdl.app.SDLActivity;

public class RetroActivity extends SDLActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        DisplayMetrics dm = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(dm);
        tellScale(dm.xdpi / 160.0f);
    }

    private static native void tellScale(float scale);
}
