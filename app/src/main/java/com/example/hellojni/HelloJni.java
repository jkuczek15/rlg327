package com.example.hellojni;

import android.app.Activity;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.os.Bundle;

public class HelloJni extends Activity
{
    TextView tv1;
    TextView tv2;
    TextView tv3;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main_activity);
        tv1 = (TextView) findViewById(R.id.textView);
        tv2 = (TextView) findViewById(R.id.textView2);
        tv3 = (TextView) findViewById(R.id.textView3);

        createDungeon();
        tv1.setText(dungeonText());
        tv3.setText("HP: " + getHp());
        tv1.setTextSize(9);
        tv2.setTextSize(20);
        int[] buttons = {R.id.one, R.id.two, R.id.three, R.id.four, R.id.five, R.id.six, R.id.seven,
                R.id.eight, R.id.nine, R.id.stairs_down, R.id.stairs_up, R.id.restart};

        for (int i = 0; i < buttons.length; i++) {
            Button buttonNum = (Button) findViewById(buttons[i]);
            buttonNum.setOnClickListener(onClick);
        }
    }

    private View.OnClickListener onClick= new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            switch(v.getId()) {
                case R.id.one:
                    handleInput1();
                    break;
                case R.id.two:
                    handleInput2();
                    break;
                case R.id.three:
                    handleInput3();
                    break;
                case R.id.four:
                    handleInput4();
                    break;
                case R.id.five:
                    handleInput5();
                    break;
                case R.id.six:
                    handleInput6();
                    break;
                case R.id.seven:
                    handleInput7();
                    break;
                case R.id.eight:
                    handleInput8();
                    break;
                case R.id.nine:
                    handleInput9();
                    break;
                case R.id.stairs_up:
                    handleInputStairsUp();
                    break;
                case R.id.stairs_down:
                    handleInputStairsDown();
                    break;
                case R.id.restart:
                    handleInputRestart();
                    break;

            }

            doMoves();
            tv1.setText(dungeonText());
            tv2.setText(getCombatMessage());
            tv3.setText("HP: " + getHp());
        }
    };

    /* Returns the entire dungeon as a Java String
     * so we can display it in the .apk */
    public native String dungeonText();

    public native int getHp();

    public native void createDungeon();

    public native void handleInput1();

    public native void handleInput2();

    public native void handleInput3();

    public native void handleInput4();

    public native void handleInput5();

    public native void handleInput6();

    public native void handleInput7();

    public native void handleInput8();

    public native void handleInput9();

    public native void handleInputStairsUp();

    public native void handleInputStairsDown();

    public native void doMoves();

    public native void handleInputRestart();

    public native String getCombatMessage();

    /* this is used to load the 'hello-jni' library on application
     * startup. The library has already been unpacked into
     * /data/data/com.example.hellojni/lib/libhello-jni.so at
     * installation time by the package manager.
     */
    static {
        System.loadLibrary("hello-jni");
    }

}
