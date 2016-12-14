package com.example.pcsc_sdk_example;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.ftsafe.pcsclite.PCSC;
import com.ftsafe.pcsclite.PCSCException;
import com.ftsafe.pcsclite.Pcscd;
import com.ftsafe.usb.Utility;

import android.R.integer;
import android.accounts.Account;
import android.accounts.OnAccountsUpdateListener;
import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.Editable;
import android.text.TextWatcher;
import android.text.method.ScrollingMovementMethod;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.WindowManager;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

public class MainActivity extends Activity {

	private TextView textView;
	private Button button11;
	private Button button12;
	private Button button13;
	private Button button14;
	private Button button15;
	
	private Button button21;
	private Button button22;
	private Button button23;
	private Button button24;
	
	private Button button31;
	private Button button32;
	private Button button33;
	private Button button34;
	
	private Button button41;
	private Button button42;
	private Button button43;
	private Button button44;
	
	
	private Button button51;
	private EditText editText52;
	
	private Button button61;
	private EditText editText62;
	
	private Context context = this;
	private Pcscd pcscd = null;
	private PCSC pcsc = null;
	
	private long contextId;
	private long cardId;
	
	private int cardProtocol;
	
	
	
	private Handler mHandler = new Handler(){
		public void handleMessage(Message msg){
			super.handleMessage(msg);			
			textView.append(msg.obj.toString());
		}
	};
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		
		
		
		textView = (TextView)findViewById(R.id.textview);
		button11 = (Button)findViewById(R.id.button11);
		button12 = (Button)findViewById(R.id.button12);
		button13 = (Button)findViewById(R.id.button13);
		button14 = (Button)findViewById(R.id.button14);
		button15 = (Button)findViewById(R.id.button15);
		
		button21 = (Button)findViewById(R.id.button21);
		button22 = (Button)findViewById(R.id.button22);
		button23 = (Button)findViewById(R.id.button23);
		button24 = (Button)findViewById(R.id.button24);
		
		button31 = (Button)findViewById(R.id.button31);
		button32 = (Button)findViewById(R.id.button32);
		button33 = (Button)findViewById(R.id.button33);
		button34 = (Button)findViewById(R.id.button34);
		
		button41 = (Button)findViewById(R.id.button41);
		button42 = (Button)findViewById(R.id.button42);
		button43 = (Button)findViewById(R.id.button43);
		button44 = (Button)findViewById(R.id.button44);
		
		button51 = (Button)findViewById(R.id.button51);
		editText52 = (EditText)findViewById(R.id.editText52);
		
		button61 = (Button)findViewById(R.id.button61);
		editText62 = (EditText)findViewById(R.id.editText62);
		
		textView.setMovementMethod(ScrollingMovementMethod.getInstance());
		
		
		
		
		button11.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				textView.setText("");
			}
		});
		
		button12.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				showLog("*******************************************************");
				showLog(getApplicationContext().getFilesDir().getAbsolutePath());
				showLog("Show the service status..");
				showLog(Utility.exec("ps | grep android_pcscd"));
				showLog("*******************************************************");
				
			}
		});
		
		button13.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				if(pcscd != null){
					showLog("start pcscd ...");
					pcscd.startProcess();
				}else{
					showLog("pscsd == null");
				}
					
			}
		});
		
		button14.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				if(pcscd != null){
					showLog("stop pcscd ...");
					pcscd.stopProcess();
				}else{
					showLog("pscsd == null");
				}
			}
		});
		
		
		button15.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View arg0) {
				System.exit(0);
			}
		});
		
		
		button21.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				try {
					contextId = PCSC.SCardEstablishContext(PCSC.SCARD_SCOPE_USER);
					showLog("SCardEstablishContext========"+Long.toHexString(contextId));
				} catch (PCSCException e) {
					showLog(e.getMessage());
				}
			}
		});
		
		button22.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				try {
					PCSC.SCardReleaseContext(contextId);
					showLog("SCardReleaseContext==OK");
				} catch (Exception e) {
					showLog(e.getMessage());
				}
			}
		});
		
		button23.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				try {
					PCSC.SCardIsValidContext(contextId);
					showLog("SCardIsValidContext=====OK");
				} catch (Exception e) {
					showLog(e.getMessage());
				}
			}
		});
		
		button24.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				try {
					String[] readers = PCSC.SCardListReaders(contextId);
					for(int i=0;i<readers.length;i++){
						showLog("reader["+i+"]==="+readers[i]);
					}
				} catch (Exception e) {
					showLog(e.getMessage());
				}
			}
		});
	
		button31.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				try {
					String[] readers = PCSC.SCardListReaders(contextId);					
					cardId = PCSC.SCardConnect(contextId, readers[0], PCSC.SCARD_SHARE_SHARED, PCSC.SCARD_PROTOCOL_T0 | PCSC.SCARD_PROTOCOL_T1);
					showLog("SCardConnect====="+Long.toHexString(cardId));
					showLog("Before call send or SCardTransmit, please call SCardStatus at first =====");
				} catch (Exception e) {
					showLog(e.getMessage());
				}
			}
		});
		
		button32.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				try {
					PCSC.SCardDisconnect(cardId, PCSC.SCARD_LEAVE_CARD);
					showLog("SCardDisconnect===OK");
				} catch (Exception e) {
					showLog(e.getMessage());
				}
			}
		});
		
		button33.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				try {
					byte[] sbuf = new byte[]{0x00, (byte) 0x84, 0x00, 0x00, 0x08};
					showLog("The default APDU is get random number from card '0084000008'");
					byte[] rbuf = PCSC.SCardTransmit(cardId, cardProtocol, sbuf, 0, sbuf.length);
					showLog("SCardTransmit==="+Utility.byte2HexStr(rbuf, rbuf.length));
				} catch (Exception e) {
					showLog("SCardTransmit=== ERROR, protocol is "+cardProtocol);
					showLog(e.getMessage());
				}
			}
		});
		
		button34.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				try {
					//byte[] sbuf = new byte[]{(byte) 0xa5,0x5a,0x32,0x31};
					byte[] sbuf = new byte[]{(byte) 0x5a,(byte)0xa5,0x20};
					byte[] rbuf = PCSC.SCardControl(cardId, 0x42000000 + 0x330008, sbuf, sbuf.length);
					showLog("SCardControl==="+Utility.byte2HexStr(rbuf, rbuf.length));
				} catch (Exception e) {
					showLog(e.getMessage());
				}
			}
		});
		
		button41.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				try {
					byte[] status = new byte[2];
					byte[] atr = PCSC.SCardStatus(cardId, status);
					cardProtocol = (int) status[1];
					showLog("ATR:"+Utility.byte2HexStr(atr, atr.length));
					showLog("state======="+Integer.toHexString((int)status[0]));
					showLog("protocol===="+Integer.toHexString((int)status[1]));
				} catch (Exception e) {
					showLog(e.getMessage());
				}
			}
		});
		
		button42.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				try {
					
					
					String[] readerNames = new String[1];
					int[] currentState = new int[1];
					int[] eventState = new int[1];
					byte[][] rgbAtr = new byte[][]{null};
					
					readerNames[0] = PCSC.SCardListReaders(contextId)[0];
					
					
					{
						currentState[0] = PCSC.SCARD_STATE_UNAWARE;
						PCSC.SCardGetStatusChange(contextId, PCSC.TIMEOUT_INFINITE, readerNames, currentState, eventState, rgbAtr);
					}
					
					if((eventState[0] & PCSC.SCARD_STATE_EMPTY) == PCSC.SCARD_STATE_EMPTY){
						
						currentState[0] = PCSC.SCARD_STATE_EMPTY;
						eventState[0] = PCSC.SCARD_STATE_PRESENT;
						PCSC.SCardGetStatusChange(contextId, PCSC.TIMEOUT_INFINITE, readerNames, currentState, eventState, rgbAtr);
					}else{
						currentState[0] = PCSC.SCARD_STATE_PRESENT;
						eventState[0] = PCSC.SCARD_STATE_EMPTY;
						PCSC.SCardGetStatusChange(contextId, PCSC.TIMEOUT_INFINITE, readerNames, currentState, eventState, rgbAtr);
					}
					
					showLog("change.....");
					
				} catch (Exception e) {
					showLog(e.getMessage());
				}
			}
		});
		
		button43.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				try {
					PCSC.SCardBeginTransaction(cardId);
					showLog("SCardBeginTransaction=======OK");
				} catch (Exception e) {
					showLog(e.getMessage());
				}
			}
		});
		
		button44.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				try {
					PCSC.SCardEndTransaction(cardId, PCSC.SCARD_LEAVE_CARD);
					showLog("SCardEndTransaction====OK");
				} catch (Exception e) {
					showLog(e.getMessage());
				}
			}
		});
		
		button51.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				try {
					
					if(editText52.getText().length() == 0){
						return;
					}					
					if(editText52.getText().length()%2 != 0){
						editText52.append("0");
					}
					String sendApdu = editText52.getText().toString();
					showLog("Tsend:"+sendApdu.toUpperCase());
					
					byte[] sendByte = Utility.hexStringToBytes(sendApdu);
					
					byte[] recvByte = PCSC.SCardTransmit(cardId, cardProtocol, sendByte, 0, sendByte.length);
					
					String recvApdu = Utility.byte2HexStr(recvByte, recvByte.length);
					showLog("Trecv:"+recvApdu.toUpperCase());
					
					
				} catch (Exception e) {
					showLog(e.getMessage());
				}
			}
		});
		
		editText52.addTextChangedListener(new TextWatcher() {
			@Override
			public void onTextChanged(CharSequence arg0, int arg1, int arg2, int arg3) {
				String editable = editText52.getText().toString();
				String str = StringFilter(editable.toString());
				if(!editable.equals(str)){
					editText52.setText(str);
					editText52.setSelection(str.length());
				}
			}
			
			@Override
			public void beforeTextChanged(CharSequence arg0, int arg1, int arg2, int arg3) {
			}
			
			@Override
			public void afterTextChanged(Editable arg0) {
			}
			
			private String StringFilter(String str){
				String regEx = "[^a-fA-F0-9]";
				Pattern p = Pattern.compile(regEx);
				Matcher m = p.matcher(str);
				return m.replaceAll("").trim();
			}
		});
		
		button61.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View arg0) {
				try {
					if(editText62.getText().length() == 0){
						return;
					}					
					if(editText62.getText().length()%2 != 0){
						editText62.append("0");
					}
					String sendApdu = editText62.getText().toString();
					showLog("Csend:"+sendApdu.toUpperCase());
					
					byte[] sendByte = Utility.hexStringToBytes(sendApdu);

					byte[] recvByte = PCSC.SCardControl(cardId, 0x42000000 + 0x330008, sendByte, sendByte.length);
					
					String recvApdu = Utility.byte2HexStr(recvByte, recvByte.length);
					showLog("Trecv:"+recvApdu.toUpperCase());
				} catch (Exception e) {
					showLog(e.getMessage());
				}
			}
		});
		
		editText62.addTextChangedListener(new TextWatcher() {
			@Override
			public void onTextChanged(CharSequence arg0, int arg1, int arg2, int arg3) {
				String editable = editText62.getText().toString();
				String str = StringFilter(editable.toString());
				if(!editable.equals(str)){
					editText62.setText(str);
					editText62.setSelection(str.length());
				}
			}
			
			@Override
			public void beforeTextChanged(CharSequence arg0, int arg1, int arg2, int arg3) {
			}
			
			@Override
			public void afterTextChanged(Editable arg0) {
			}
			
			private String StringFilter(String str){
				String regEx = "[^a-fA-F0-9]";
				Pattern p = Pattern.compile(regEx);
				Matcher m = p.matcher(str);
				return m.replaceAll("").trim();
			}
		});
		
		try {
			
			//32bit bluetooth or usb
			pcscd = new Pcscd(context,"android_pcscd_x32_0.1",Pcscd.bluetooth);
			//pcscd = new Pcscd(context,"android_pcscd_x32_0.1",Pcscd.usb);
			
			//64bit bluetooth or usb
			//pcscd = new Pcscd(context,"android_pcscd_x64_0.1",Pcscd.bluetooth);
			//pcscd = new Pcscd(context,"android_pcscd_x64_0.1",Pcscd.usb);
			pcsc = new PCSC(context);
		} catch (Exception e) {
			e.printStackTrace();
		}
		

		
		getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_HIDDEN);
		
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}
	
	public void showLog(String log){
		mHandler.sendMessage(mHandler.obtainMessage(0, log+"\n"));
	}
	
}
