// Ana sayfa (tek sayfa): ekran metni, resim (fill komutlarıyla), WiFi ayarı, .tft yükleme. Resim: tarayıcı küçültüp renk azaltır, ESP-01 yalnızca iletir.
#pragma once
#include <Arduino.h>

static const char ANA_HTML[] PROGMEM = R"HTML(<!doctype html><meta charset=utf-8>
<meta name=viewport content="width=device-width,initial-scale=1"><title>NX Hub</title>
<link rel=stylesheet href=/pano.css>
<style>
main{max-width:560px;margin:16px auto;padding:0 16px}
.card{padding:16px 18px 18px;margin-bottom:16px}
#dz{border:2px dashed #5b616c;border-radius:8px;padding:26px 12px;text-align:center;cursor:pointer;margin-top:12px;color:#9ca3af}
#dz.on{border-color:#60a5fa;color:#e5e7eb}
canvas{width:100%;background:#000;border:2px solid #000;margin:12px 0;image-rendering:pixelated}
.st{margin-top:10px;color:#9ca3af;min-height:1.4em;font-family:Consolas,monospace;font-size:13px}
.etiket{display:inline-block;width:100px;color:#9ca3af}
.satir{display:flex;gap:8px;align-items:center;flex-wrap:wrap}
</style>
<div class=bar><b>NX Hub</b><span>WiFi: <span id=wifi>-</span></span><span id=srm>-</span></div>
<div class=nav style="display:flex;gap:6px;padding:10px 16px;flex-wrap:wrap">
<button class=sekme-btn data-t=ekran>Ekran</button><button class=sekme-btn data-t=resim>Resim</button><button class=sekme-btn data-t=wifi>WiFi</button><button class=sekme-btn data-t=dosya>Ekran tasarimi</button></div>
<main>
<div class=card id=c_ekran><h3>Ekran</h3>
<div class=satir style="margin-top:12px"><input id=dt maxlength=30 placeholder="ekran metni (bos = IP + surum)" style="flex:1;padding:10px;border-radius:8px;font-size:15px"><button class="btn btn-mavi" onclick="yaz()">Gonder</button></div>
<div class=satir><input id=sh placeholder="sehir (or. Ankara)" style="flex:1;padding:10px;border-radius:8px;font-size:15px"><button class="btn btn-mavi" onclick="sehir()">Sehir ayarla</button><button class="btn btn-turuncu" onclick="f('/api/yenile')">Yenile</button></div>
<p style="margin:12px 0 0;line-height:1.7"><span class=etiket>Saat</span><span id=vs>-</span><br><span class=etiket>Sehir</span><span id=vh>-</span><br><span class=etiket>Hava</span><span id=vd>-</span><br><span class=etiket>Haber</span><span id=vn>-</span><br><span class=etiket>Haber durumu</span><span id=vb>-</span><br><span class=etiket>Son metin</span><span id=dm>-</span><br><span class=etiket>Son komut</span><span id=nx>-</span></p></div>
<div class=card id=c_resim><h3>Resim gonder</h3>
<div id=dz>Resmi buraya birak ya da tikla</div>
<input id=fi type=file accept="image/*" hidden>
<canvas id=pv width=480 height=320></canvas>
<div class=satir>Blok <select id=bs><option>2<option selected>3<option>4<option>6</select>
<button id=go class="btn btn-yesil" disabled>Gonder</button><button id=rs class="btn btn-mavi">Ekrani yenile</button></div>
<div id=st class=st></div></div>
<div class=card id=c_wifi><h3>WiFi</h3>
<p style="margin:12px 0 0;line-height:1.7"><span class=etiket>Ag</span><b id=ws>-</b><br><span class=etiket>IP</span><span id=wi>-</span><br><span class=etiket>Sinyal</span><span id=wr>-</span></p>
<div class=satir style="margin-top:12px"><button id=wd class="btn btn-mavi">Ag degistir</button><button id=wx class="btn btn-kirmizi">Kurulum moduna al</button></div>
<div id=wp hidden style="margin-top:12px"><select id=wsel style="width:100%;padding:10px"></select>
<div class=satir><input id=wpw type=password placeholder="sifre (acik agda bos)" style="flex:1;padding:10px;border-radius:8px;font-size:15px"><button id=wb class="btn btn-yesil">Baglan</button></div></div>
<div id=st3 class=st></div></div>
<div class=card id=c_dosya><h3>Ekran tasarimi yukle (.tft)</h3>
<p style="margin:10px 0 0;color:#9ca3af;font-size:13px">Resim degil: Nextion Editor'den cikan ekran tasarimi. Yuklenince ekrandaki tum tasarim degisir. Fotograf icin Resim sekmesini kullan.</p>
<div id=dz2>Tasarim dosyasini (.tft) buraya birak ya da tikla</div>
<input id=ft type=file accept=".tft" hidden>
<div class=satir style="margin-top:12px"><span id=tn style="flex:1;color:#9ca3af">dosya secilmedi</span>
<button id=tg class="btn btn-turuncu" disabled>Yukle</button></div>
<div id=st2 class=st></div></div>
</main>
<script>
const W=480,H=320,$=i=>document.getElementById(i);
let img=null,cmds=[];
const st=t=>$('st').textContent=t;
$('dz').onclick=()=>$('fi').click();
$('dz').ondragover=e=>{e.preventDefault();$('dz').classList.add('on')};
$('dz').ondragleave=()=>$('dz').classList.remove('on');
$('dz').ondrop=e=>{e.preventDefault();$('dz').classList.remove('on');load(e.dataTransfer.files[0])};
$('fi').onchange=e=>load(e.target.files[0]);
$('bs').onchange=()=>img&&render();
function load(f){if(!f)return;const i=new Image();i.onload=()=>{img=i;render()};i.src=URL.createObjectURL(f)}
function render(){
  const bs=+$('bs').value,w=W/bs|0,h=H/bs|0;
  const o=document.createElement('canvas');o.width=w;o.height=h;
  const x=o.getContext('2d');x.fillStyle='#000';x.fillRect(0,0,w,h);
  const s=Math.min(w/img.width,h/img.height),dw=img.width*s,dh=img.height*s;
  x.drawImage(img,(w-dw)/2,(h-dh)/2,dw,dh);
  const d=x.getImageData(0,0,w,h),p=d.data,C=new Array(w*h);
  for(let i=0;i<w*h;i++){
    const r=p[i*4]>>4,g=p[i*4+1]>>4,b=p[i*4+2]>>4;
    const r5=(r<<1)|(r>>3),g6=(g<<2)|(g>>2),b5=(b<<1)|(b>>3);
    C[i]=(r5<<11)|(g6<<5)|b5;
    p[i*4]=r*17;p[i*4+1]=g*17;p[i*4+2]=b*17;
  }
  x.putImageData(d,0,0);
  const c=$('pv').getContext('2d');c.imageSmoothingEnabled=false;c.drawImage(o,0,0,w*bs,h*bs);
  const open=new Map(),out=[];
  for(let y=0;y<h;y++){let xx=0;while(xx<w){
    const col=C[y*w+xx];let x2=xx;while(x2+1<w&&C[y*w+x2+1]==col)x2++;
    const k=xx+','+(x2-xx+1)+','+col,r=open.get(k);
    if(r&&r.y+r.h==y)r.h++;else{const n={x:xx,y:y,w:x2-xx+1,h:1,c:col};open.set(k,n);out.push(n)}
    xx=x2+1}}
  cmds=['cls 0'].concat(out.map(r=>'fill '+r.x*bs+','+r.y*bs+','+r.w*bs+','+r.h*bs+','+r.c));
  st(cmds.length+' komut, ~'+Math.round(cmds.join('\n').length/1024)+' KB');
  $('go').disabled=false;
}
$('go').onclick=()=>{
  const fd=new FormData();fd.append('cmd',new Blob([cmds.join('\n')+'\n']),'foto.txt');
  const x=new XMLHttpRequest();x.open('POST','/foto');
  x.upload.onprogress=e=>st('gonderiliyor %'+Math.round(100*e.loaded/e.total));
  x.onload=()=>st(x.responseText);x.onerror=()=>st('baglanti hatasi');
  $('go').disabled=true;x.onloadend=()=>$('go').disabled=false;x.send(fd);
};
$('rs').onclick=()=>fetch('/nx?c='+encodeURIComponent('page 0')).then(()=>st('ekran yenilendi'));
let tf=null;const st2=t=>$('st2').textContent=t;
const pick=f=>{if(!f)return;tf=f;$('tn').textContent=f.name+' ('+Math.round(f.size/1024)+' KB)';$('tg').disabled=false;st2('')};
$('dz2').className='';$('dz2').style.cssText=$('dz').style.cssText;
$('dz2').setAttribute('style','border:2px dashed #5b616c;border-radius:8px;padding:26px 12px;text-align:center;cursor:pointer;color:#9ca3af');
$('dz2').onclick=()=>$('ft').click();
$('dz2').ondragover=e=>{e.preventDefault()};
$('dz2').ondrop=e=>{e.preventDefault();pick(e.dataTransfer.files[0])};
$('ft').onchange=e=>pick(e.target.files[0]);
$('tg').onclick=()=>{
  if(!tf||!confirm('Ekrandaki mevcut tasarim silinip bu dosya yuklenecek. Devam?'))return;
  const fd=new FormData();fd.append('tft',tf,tf.name);
  const x=new XMLHttpRequest();x.open('POST','/tft?size='+tf.size);
  x.upload.onprogress=e=>st2('gonderiliyor %'+Math.round(100*e.loaded/e.total));
  x.onload=()=>st2(x.responseText);x.onerror=()=>st2('baglanti hatasi');
  $('tg').disabled=true;x.onloadend=()=>$('tg').disabled=false;x.send(fd);
};
function g(d){dm.textContent=d.durum;nx.textContent=d.nx;wifi.textContent=d.ssid+' ('+d.ip+')';srm.textContent=d.surum+' • Son güncelleme: '+d.tarih;
vs.textContent=d.saat;vh.textContent=d.sehir+' '+d.hava+' °C';vd.textContent=d.havad;vn.textContent=d.haber||'-';vb.textContent=d.haberd+' (bellek '+d.heap+')';
ws.textContent=d.ssid||'-';wi.textContent=d.ip;wr.textContent=d.rssi+' dBm'}
function f(u){fetch(u).then(r=>r.json()).then(g).catch(()=>nx.textContent='baglanti yok')}
function sehir(){f('/api/sehir?ad='+encodeURIComponent(sh.value))}
function yaz(){f('/api/durum?t='+encodeURIComponent(dt.value))}
dt.onkeydown=e=>{if(e.key=='Enter')yaz()};f('/api/durum');setInterval(()=>f('/api/durum'),15000);
function tara(){$('st3').textContent='aglar taraniyor...';fetch('/api/tara').then(r=>r.json()).then(d=>{
  if(d.durum){setTimeout(tara,1500);return}
  const sel=$('wsel');sel.innerHTML='';
  d.aglar.forEach(a=>{const o=document.createElement('option');o.value=a.s;o.textContent=a.s+' ('+a.r+' dBm'+(a.a?'':', acik')+')';sel.appendChild(o)});
  $('st3').textContent=d.aglar.length+' ag bulundu'}).catch(()=>$('st3').textContent='tarama hatasi')}
$('wd').onclick=()=>{$('wp').hidden=false;tara()};
$('wb').onclick=()=>{const s=$('wsel').value;if(!s)return;$('st3').textContent='baglaniyor (en cok 30 sn)...';
  fetch('/api/wifikaydet?s='+encodeURIComponent(s)+'&p='+encodeURIComponent($('wpw').value)).then(r=>r.json()).then(d=>{
    if(d.ok){$('st3').textContent='baglandi: '+d.ip+' (adres degistiyse http://'+d.ip+')';f('/api/durum')}
    else $('st3').textContent='baglanamadi, eski ag geri yuklendi'}).catch(()=>$('st3').textContent='cevap yok (ag degismis olabilir, yeni IP ile dene)')};
$('wx').onclick=()=>{
  if(!confirm('Kayitli WiFi silinecek, cihaz yeniden baslayip NXHub-Kurulum agini acacak. Telefonla o aga baglanip yeni agi secmen gerekir. Devam?'))return;
  fetch('/api/wifisil').then(()=>$('st3').textContent='silindi, cihaz yeniden basliyor: NXHub-Kurulum agina baglan').catch(()=>$('st3').textContent='silindi, cihaz yeniden basliyor')};
function sek(n){document.querySelectorAll('main>.card').forEach(c=>c.hidden=c.id!='c_'+n);
  document.querySelectorAll('.sekme-btn').forEach(b=>b.classList.toggle('aktif',b.dataset.t==n));
  try{localStorage.setItem('sek',n)}catch(e){}}
document.querySelectorAll('.sekme-btn').forEach(b=>b.onclick=()=>sek(b.dataset.t));
let sk='ekran';try{sk=localStorage.getItem('sek')||'ekran'}catch(e){}
sek(document.getElementById('c_'+sk)?sk:'ekran');
if(location.hostname=='192.168.4.1')sek('wifi');
</script>)HTML";
