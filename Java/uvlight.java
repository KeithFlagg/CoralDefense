package com.example.arduinocontrol;

import androidx.appcompat.app.AppCompatActivity;
import java.io.IOException;
import java.util.Set;
import java.util.UUID;
import android.annotation.SuppressLint;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

public class uvlight extends AppCompatActivity implements OnClickListener{
    private static final int REQUEST_ENABLE_BT = 1;
    //listeners for button presses
    Button on, off;
    TextView t1;

    String address = null , name=null;
    BluetoothAdapter myBluetooth = null;
    BluetoothSocket btSocket = null;
    Set<BluetoothDevice> pairedDevices;
    static final UUID myUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        //Instantiation
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_uvlight);
        try {setw();} catch (Exception e) {}
    }

    @SuppressLint("ClickableViewAccessibility")
    private void setw() throws IOException
    {
        //text for bluetooth device details
        t1=(TextView)findViewById(R.id.textView3);
        bluetooth_connect_device();


        //links on and off to their element IDs
        on=(Button)findViewById(R.id.uvlighton);
        off=(Button)findViewById(R.id.uvlightoff);
        
        on.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View v)
            {
                //sends 'o' char to Arduino
                // method to turn on uv light
                led_on_off('o');
            }
        });

        off.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                //method to turn off uv light
                led_on_off('f');
            }
        });

    }

    private void bluetooth_connect_device() throws IOException
    {
        try
        {
            //initializes the bluetooth adapter
            myBluetooth = BluetoothAdapter.getDefaultAdapter();
            //bluetooth MAC address
            address = myBluetooth.getAddress();
            //Gets paired device name
            pairedDevices = myBluetooth.getBondedDevices();
            if (!myBluetooth.isEnabled()) {
                //requests user to enable bluetooth
                Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                //calls activity to request the user to enable bluetooth
                startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
            }
            if (pairedDevices.size()>0)
            {
                for(BluetoothDevice bt : pairedDevices)
                {
                    //converts address to a string
                    address=bt.getAddress().toString();name = bt.getName().toString();
                    Toast.makeText(getApplicationContext(),"Connected to your Arduino", Toast.LENGTH_SHORT).show();

                }
            }

        }
        catch(Exception we){}
        myBluetooth = BluetoothAdapter.getDefaultAdapter();
        //get the bluetooth device
        BluetoothDevice connected = myBluetooth.getRemoteDevice(address);
        //connects to the device's address and checks if it's available
        btSocket = connected.createInsecureRfcommSocketToServiceRecord(myUUID);
        //create a RFCOMM (SPP) connection
        btSocket.connect();
        //changes text to display bluetooth name and address and catches the error
        try { t1.setText("BT Name: "+name+"\nBT Address: "+address); }
        catch(Exception e){}
    }

    @Override
    public void onClick(View v)
    {
        try
        {

        }
        catch (Exception e)
        {
            //displays toast message
            Toast.makeText(getApplicationContext(),e.getMessage(), Toast.LENGTH_SHORT).show();

        }

    }

    private void led_on_off(char i)
    {
        try
        {
            if (btSocket!=null)
            {
                //sends char to Arduino
                btSocket.getOutputStream().write(i);
            }

        }
        catch (Exception e)
        {
            Toast.makeText(getApplicationContext(),e.getMessage(), Toast.LENGTH_SHORT).show();

        }

    }

}

