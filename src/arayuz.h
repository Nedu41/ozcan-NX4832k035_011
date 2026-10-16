// Ana sayfa (tek sayfa): ekran mesajı/renk/hız/animasyon, WiFi ayarı, .tft yükleme.
#pragma once
#include <Arduino.h>

static const char ANA_HTML[] PROGMEM = R"HTML(<!doctype html><meta charset=utf-8>
<meta name=viewport content="width=device-width,initial-scale=1"><title>NX Hub</title>
<link rel=stylesheet href=/pano.css>
<style>
.pg{display:flex;gap:6px;flex-wrap:wrap;align-items:center;margin-top:6px}.pg small{width:52px;color:#9ca3af}
.sw{width:34px;height:34px;border-radius:8px;border:2px solid #000;cursor:pointer;padding:0}
.sw.on{outline:2px solid #e5e7eb;outline-offset:2px}
main{max-width:560px;margin:16px auto;padding:0 16px}
.card{padding:16px 18px 18px;margin-bottom:16px}
.st{margin-top:10px;color:#9ca3af;min-height:1.4em;font-family:Consolas,monospace;font-size:13px}
.etiket{display:inline-block;width:100px;color:#9ca3af}
.satir{display:flex;gap:8px;align-items:center;flex-wrap:wrap}
</style>
<div class=bar><b>NX Hub</b><span>WiFi: <span id=wifi>-</span></span><span id=srm>-</span></div>
<div class=nav style="display:flex;gap:6px;padding:10px 16px;flex-wrap:wrap">
<button class=sekme-btn data-t=ekran>Ekran</button><button class=sekme-btn data-t=wifi>WiFi</button><button class=sekme-btn data-t=dosya>Ekran tasarimi</button></div>
<main>
<div class=card id=c_ekran><h3>Ekran</h3>
<div class=satir style="margin-top:12px"><input id=dt maxlength=300 placeholder="kayar yazi (g1, en cok 300 karakter)" style="flex:1;padding:10px;border-radius:8px;font-size:15px"><button class="btn btn-mavi" onclick="yaz()">Gonder</button></div>
<div id=pal style="margin-top:10px"></div>
<div id=arkapal style="margin-top:10px"></div>
<div id=syfpal style="margin-top:10px"></div>
<div class=satir style="margin-top:10px"><span class=etiket>Karakter</span><select id=kk style="flex:1;padding:10px"><option value=0>ASCII (Turkce harfler sadelesir)<option value=1>ISO-8859-9 (Turkce font)<option value=2>UTF-8</select></div>
<div class=satir style="margin-top:10px"><span class=etiket>Animasyon</span><select id=an style="flex:1;padding:10px"><option value=0>Sabit<option value=1>Yanip sonen<option value=2>Gokkusagi<option value=3>Nefes</select></div>
<div class=satir style="margin-top:10px"><span class=etiket>Kayma hizi</span><input id=hz type=range min=1 max=10 value=3 style="flex:1"><b id=hzv>3</b></div>
<div class=satir><input id=sh placeholder="sehir (or. Ankara)" style="flex:1;padding:10px;border-radius:8px;font-size:15px"><button class="btn btn-mavi" onclick="sehir()">Sehir ayarla</button><button class="btn btn-turuncu" onclick="f('/api/yenile')">Yenile</button></div>
<p style="margin:12px 0 0;line-height:1.7"><span class=etiket>Saat</span><span id=vs>-</span><br><span class=etiket>Sehir</span><span id=vh>-</span><br><span class=etiket>Hava</span><span id=vd>-</span><br><span class=etiket>Haber</span><span id=vn>-</span><br><span class=etiket>Haber durumu</span><span id=vb>-</span><br><span class=etiket>t_durum</span><span id=dm>-</span><br><span class=etiket>Son komut</span><span id=nx>-</span></p></div>
<div class=card id=c_wifi><h3>WiFi</h3>
<p style="margin:12px 0 0;line-height:1.7"><span class=etiket>Ag</span><b id=ws>-</b><br><span class=etiket>IP</span><span id=wi>-</span><br><span class=etiket>Sinyal</span><span id=wr>-</span></p>
<div class=satir style="margin-top:12px"><button id=wd class="btn btn-mavi">Ag degistir</button><button id=wx class="btn btn-kirmizi">Kurulum moduna al</button></div>
<div id=wp hidden style="margin-top:12px"><select id=wsel style="width:100%;padding:10px"></select>
<div class=satir><input id=wpw type=password placeholder="sifre (acik agda bos)" style="flex:1;padding:10px;border-radius:8px;font-size:15px"><button id=wb class="btn btn-yesil">Baglan</button></div></div>
<div id=st3 class=st></div></div>
<div class=card id=c_dosya><h3>Ekran tasarimi yukle (.tft)</h3>
<p style="margin:10px 0 0;color:#9ca3af;font-size:13px">Nextion Editor'den cikan ekran tasarimi dosyasi. Yuklenince ekrandaki tum tasarim degisir.</p>
<div id=dz2>Tasarim dosyasini (.tft) buraya birak ya da tikla</div>
<input id=ft type=file accept=".tft" hidden>
<div class=satir style="margin-top:12px"><span id=tn style="flex:1;color:#9ca3af">dosya secilmedi</span>
<button id=tg class="btn btn-turuncu" disabled>Yukle</button></div>
<div id=st2 class=st></div></div>
</main>
<script>
const $=i=>document.getElementById(i);
let tf=null;const st2=t=>$('st2').textContent=t;
const pick=f=>{if(!f)return;tf=f;$('tn').textContent=f.name+' ('+Math.round(f.size/1024)+' KB)';$('tg').disabled=false;st2('')};
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
if(document.activeElement!==hz){hz.value=d.hiz;hzv.textContent=d.hiz}renk=d.renk;mark();arka=d.arka;arkam=d.arkam;markA();syf=d.sayfa;markS();document.getElementById('uy').checked=d.uydur==1;if(document.activeElement!==an)an.value=d.anim;if(document.activeElement!==kk)kk.value=d.tr;
ws.textContent=d.ssid||'-';wi.textContent=d.ip;wr.textContent=d.rssi+' dBm'}
function f(u){fetch(u).then(r=>r.json()).then(g).catch(()=>nx.textContent='baglanti yok')}
function sehir(){f('/api/sehir?ad='+encodeURIComponent(sh.value))}
const GRUP=[['Pastel',['#DADAE4','#7FE0DC','#FFB27A','#8FE0A0','#FF9A9A','#C9A7FF','#FFE08A','#8FB8FF','#F4A6D7']],
['Ana',['#FFFFFF','#FF0000','#00FF00','#3C64FF','#FFFF00','#FF8000','#00FFFF','#FF00FF']],
['Neon',['#39FF14','#04D9FF','#FF1493','#CCFF00','#FF5F1F','#B026FF','#FF073A','#0FF0B3']]];
const h565=x=>{const n=parseInt(x.slice(1),16),r=(n>>16)&255,g=(n>>8)&255,b=n&255;return((r>>3)<<11)|((g>>2)<<5)|(b>>3)};
let renk=h565(GRUP[0][1][0]);
GRUP.forEach(([ad,liste])=>{const r=document.createElement('div');r.className='pg';r.innerHTML='<small>'+ad+'</small>';
  liste.forEach(x=>{const b=document.createElement('button');b.className='sw';b.style.background=x;b.dataset.v=h565(x);b.onclick=()=>{renk=+b.dataset.v;mark();f('/api/mesaj?r='+renk)};r.appendChild(b)});pal.appendChild(r)});
an.onchange=()=>f('/api/anim?m='+an.value);kk.onchange=()=>f('/api/tr?m='+kk.value);
const ARKA=['#000000','#14141A','#2C2C38','#4B0F0F','#0F2F4B','#0F4B2B','#FFFFFF'];
let arka=0,arkam=0;
{const r=document.createElement('div');r.className='pg';r.innerHTML='<small>Arka</small>';
 ARKA.forEach(x=>{const b=document.createElement('button');b.className='sw sa';b.style.background=x;b.dataset.v=h565(x);b.onclick=()=>{arka=+b.dataset.v;arkam=0;markA();f('/api/arka?r='+arka)};r.appendChild(b)});
 const s=document.createElement('button');s.className='btn btn-mavi';s.id='asayfa';s.textContent='Sayfa rengi';s.onclick=()=>{arkam=1;markA();f('/api/arka?s=1')};r.appendChild(s);arkapal.appendChild(r)}
const SYF=['#000000','#14141A','#1F1F28','#2C2C38','#800000','#4B0F0F','#0F2F4B','#0F4B2B','#3A1F5C'];
let syf=-1;
{const r=document.createElement('div');r.className='pg';r.innerHTML='<small>Sayfa</small>';
 SYF.forEach(x=>{const b=document.createElement('button');b.className='sw ss';b.style.background=x;b.dataset.v=h565(x);b.onclick=()=>{syf=+b.dataset.v;markS();f('/api/sayfa?r='+syf)};r.appendChild(b)});
 const l=document.createElement('label');l.style.cssText='font-size:13px;color:#9ca3af;display:flex;gap:6px;align-items:center';l.innerHTML='<input type=checkbox id=uy> kutular da uysun';r.appendChild(l);syfpal.appendChild(r);
 document.getElementById('uy').onchange=e=>f('/api/sayfa?u='+(e.target.checked?1:0))}
function markS(){document.querySelectorAll('.ss').forEach(b=>b.classList.toggle('on',+b.dataset.v==syf))}
function markA(){document.querySelectorAll('.sa').forEach(b=>b.classList.toggle('on',arkam==0&&+b.dataset.v==arka));asayfa.classList.toggle('btn-on',arkam==1);asayfa.classList.add('sabit')}
function mark(){document.querySelectorAll('.sw').forEach(b=>b.classList.toggle('on',+b.dataset.v==renk))}
hz.oninput=()=>hzv.textContent=hz.value;hz.onchange=()=>f('/api/hiz?v='+hz.value);
function yaz(){f('/api/mesaj?t='+encodeURIComponent(dt.value)+'&r='+renk)}
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
