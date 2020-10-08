package com.example.arduinocontrol;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;

import androidx.appcompat.app.AppCompatActivity;
import androidx.cardview.widget.CardView;

public class HomeActivity extends AppCompatActivity implements View.OnClickListener {
    private CardView airPump, flowMeter, humiditySensor, levelSwitch1, levelSwitch2, lightStrip, phMeter, uvLight, waterPump, solenoidValve, nutrientPump;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_home);
        //defines cards
        airPump = (CardView) findViewById(R.id.airpump);
        flowMeter = (CardView) findViewById(R.id.flowmeter);
        humiditySensor = (CardView) findViewById(R.id.humiditysensor);
        levelSwitch1 = (CardView) findViewById(R.id.levelswitch1);
        levelSwitch2 = (CardView) findViewById(R.id.levelswitch2);
        lightStrip = (CardView) findViewById(R.id.lightstrip);
        phMeter = (CardView) findViewById(R.id.phmeter);
        uvLight = (CardView) findViewById(R.id.uvlight);
        waterPump = (CardView) findViewById(R.id.waterpump);
        solenoidValve = (CardView) findViewById(R.id.solenoidvalve);
        nutrientPump = (CardView) findViewById(R.id.nutrientpump);
        //click listeners
        airPump.setOnClickListener(this);
        flowMeter.setOnClickListener(this);
        humiditySensor.setOnClickListener(this);
        levelSwitch1.setOnClickListener(this);
        levelSwitch2.setOnClickListener(this);
        lightStrip.setOnClickListener(this);
        phMeter.setOnClickListener(this);
        uvLight.setOnClickListener(this);
        waterPump.setOnClickListener(this);
        solenoidValve.setOnClickListener(this);
        nutrientPump.setOnClickListener(this);
    }

    @Override
    public void onBackPressed() {
        moveTaskToBack(true);
    }

    @Override
    public void onClick(View v) {
        Intent i;

        //on click opens the new activity
        switch(v.getId()){
            case R.id.airpump : i = new Intent(this,Airpump.class); startActivity(i); break;
            case R.id.flowmeter : i = new Intent(this,Flowmeter.class); startActivity(i); break;
            case R.id.humiditysensor : i =new Intent(this,Humiditysensor.class); startActivity(i); break;
            case R.id.levelswitch1 : i = new Intent(this,Levelswitch1.class); startActivity(i); break;
            case R.id.levelswitch2 : i = new Intent(this,Levelswitch2.class); startActivity(i); break;
            case R.id.lightstrip : i = new Intent(this,Lightstrip.class); startActivity(i); break;
            case R.id.phmeter : i = new Intent(this,Phmeter.class); startActivity(i); break;
            case R.id.uvlight : i = new Intent(this,uvlight.class); startActivity(i); break;
            case R.id.waterpump : i = new Intent(this,Waterpump.class); startActivity(i); break;
            case R.id.solenoidvalve : i = new Intent(this,solenoidvalve.class); startActivity(i); break;
            case R.id.nutrientpump : i = new Intent(this,Nutrientpump.class); startActivity(i); break;
            default:break;
        }
    }
}
