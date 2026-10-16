// /foto: tarayıcıda resmi küçültüp renk azaltır, Nextion "fill" komutlarına çevirir; ESP-01 yalnızca iletir.
#pragma once
#include <Arduino.h>

static const char FOTO_HTML[] PROGMEM = R"HTML(<!doctype html><meta charset=utf-8>
<meta name=viewport content="width=device-width,initial-scale=1"><title>NX Foto</title>
<link rel=stylesheet href=/pano.css>
<style>
main{max-width:560px;margin:16px auto;padding:0 16px}
.card{padding:16px 18px 18px;margin-bottom:16px}
#dz{border:2px dashed #5b616c;border-radius:8px;padding:26px 12px;text-align:center;cursor:pointer;margin-top:12px;color:#9ca3af}
#dz.on{border-color:#60a5fa;color:#e5e7eb}
canvas{width:100%;background:#000;border:2px solid #000;margin:12px 0;image-rendering:pixelated}
.st{margin-top:10px;color:#9ca3af;min-height:1.4em;font-family:Consolas,monospace;font-size:13px}
.etiket{display:inline-block;width:70px;color:#9ca3af}
.satir{display:flex;gap:8px;align-items:center;flex-wrap:wrap}
</style>
<main>
<div class=card><h3>Resim gonder</h3>
<div id=dz>Resmi buraya birak ya da tikla</div>
<input id=fi type=file accept="image/*" hidden>
<canvas id=pv width=480 height=320></canvas>
<div class=satir>Blok <select id=bs><option>2<option selected>3<option>4<option>6</select>
<button id=go class="btn btn-yesil" disabled>Gonder</button><button id=rs class="btn btn-mavi">Ekrani yenile</button></div>
<div id=st class=st></div></div>
<div class=card><h3>WiFi</h3>
<p style="margin:12px 0 0;line-height:1.7"><span class=etiket>Ag</span><b id=ws>-</b><br><span class=etiket>IP</span><span id=wi>-</span><br><span class=etiket>Sinyal</span><span id=wr>-</span></p>
<div class=satir style="margin-top:12px"><a id=wl class="btn btn-mavi" style="text-decoration:none" href=#>Ag degistir</a>
<button id=wx class="btn btn-kirmizi">Kayitli agi sil</button></div>
<div id=st3 class=st></div></div>
<div class=card><h3>.tft yukle</h3>
<div id=dz2>.tft dosyasini buraya birak ya da tikla</div>
<input id=ft type=file accept=".tft" hidden>
<div class=satir style="margin-top:12px"><span id=tn style="flex:1;color:#9ca3af">dosya secilmedi</span>
<button id=tg class="btn btn-turuncu" disabled>Yukle</button></div>
<div id=st2 class=st></div></div>
<p><a href=/ style="color:#9ca3af">&larr; Ana sayfa</a></p></main>
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
$('wl').href='http://'+location.hostname+'/wifi';
fetch('/api/wifi').then(r=>r.json()).then(d=>{$('ws').textContent=d.ssid||'-';$('wi').textContent=d.ip;$('wr').textContent=d.rssi+' dBm'}).catch(()=>$('st3').textContent='bilgi alinamadi');
$('wx').onclick=()=>{
  if(!confirm('Kayitli WiFi silinecek, cihaz yeniden baslayip NXHub-Kurulum agini acacak. Telefonla o aga baglanip yeni agi secmen gerekir. Devam?'))return;
  fetch('/api/wifisil').then(()=>$('st3').textContent='silindi, cihaz yeniden basliyor: NXHub-Kurulum agina baglan').catch(()=>$('st3').textContent='silindi, cihaz yeniden basliyor');
};
</script>)HTML";
