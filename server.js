// Combined: License Server + Discord Bot
const http = require('http');
const https = require('https');
const crypto = require('crypto');
const { Client, GatewayIntentBits, EmbedBuilder, ActionRowBuilder, ButtonBuilder, ButtonStyle, ChannelType, PermissionFlagsBits } = require('discord.js');

// ====== LICENSE SERVER ======
const initialKeys = [
  { key: "4mZp9QxW2vTL", active: true, hwid: "", created: "2026-01-01", expiresAt: null },
  { key: "TestKey123", active: true, hwid: "", created: "2026-01-01", expiresAt: null },
  { key: "duaxxel", active: true, hwid: "", created: "2026-01-01", expiresAt: null },
  { key: "mmich", active: true, hwid: "", created: "2026-07-01", expiresAt: null }
];
let keys = [...initialKeys];

const SIG_SALT = "DuAxXl_S1G_SaLt_2026";
const HMAC_SECRET = "DuAxXl_M1TM_H4SH_2026";

function hmacSign(secret, data) {
  let key = secret;
  while (key.length < data.length) key += key;
  let h = 0xDEADBEEF >>> 0;
  for (let i = 0; i < data.length; i++) {
    let b = (data.charCodeAt(i) ^ key.charCodeAt(i)) >>> 0;
    h = (h ^ b) >>> 0;
    h = ((h * 33 + (b << 3)) & 0xFFFFFFFF) >>> 0;
    h = ((h << 7) | (h >>> 25)) >>> 0;
    h = (h ^ (h >>> 11)) >>> 0;
  }
  return ("0000000" + (h >>> 0).toString(16)).substr(-8);
}

function send(obj, action, res, nonce) {
  obj._n = obj._n || nonce || '0';
  obj._s = hmacSign(HMAC_SECRET, action + '|' + (obj.status || 'ok') + '|' + obj._n);
  res.end(JSON.stringify(obj));
}

function makeSig(key, hwid) {
  return crypto.createHash('sha256').update(key + '|' + hwid + '|' + SIG_SALT).digest('hex').substring(0, 16);
}

function makeCustomSig(key, hwid) {
  let buf = [];
  for (let i = 0; i < key.length && buf.length < 60; i++) buf.push(key.charCodeAt(i) ^ 0xA3);
  buf.push('|'.charCodeAt(0));
  for (let i = 0; i < hwid.length && buf.length < 120; i++) buf.push(hwid.charCodeAt(i) ^ 0xB7);
  buf.push('|'.charCodeAt(0));
  const salt = "DuAxXl_S1G_SaLt_2026";
  for (let i = 0; i < salt.length && buf.length < 180; i++) buf.push(salt.charCodeAt(i) ^ 0xC9);
  let h = 0xDEADBEEF >>> 0;
  for (let i = 0; i < buf.length; i++) {
    h = h ^ buf[i];
    h = ((h * 33 + (buf[i] << 3)) & 0xFFFFFFFF) >>> 0;
    h = ((h << 7) | (h >>> 25)) >>> 0;
    h = (h ^ (h >>> 11)) >>> 0;
  }
  return ("0000000" + h.toString(16)).substr(-8).toUpperCase() + ("0000000" + ((h ^ 0xCAFEBABE) >>> 0).toString(16)).substr(-8).toUpperCase();
}

let currentVersion = 14;
let downloadUrl = "https://github.com/kumarbot939/duaxxlem-server/raw/main/emulator.exe";
let killSwitch = false;
let adminPw = "dg7kXp9m_Admin_38";
let maintenanceMode = false;
let serverLogs = [];
const MAX_LOG = 500;
function addLog(msg) {
  const entry = `[${new Date().toISOString()}] ${msg}`;
  serverLogs.push(entry);
  if (serverLogs.length > MAX_LOG) serverLogs.shift();
}

function isExpired(keyObj) {
  if (!keyObj.expiresAt) return false;
  return new Date() > new Date(keyObj.expiresAt);
}

function daysLeft(keyObj) {
  if (!keyObj.expiresAt) return -1;
  const diff = new Date(keyObj.expiresAt) - new Date();
  return Math.max(0, Math.ceil(diff / (1000 * 60 * 60 * 24)));
}

const server = http.createServer((req, res) => {
    const url = new URL(req.url, 'http://localhost');
    const action = url.searchParams.get('action') || '';
    const nonce = url.searchParams.get('_n') || '0';
    req._n = nonce;

    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader('Content-Type', 'application/json');

    if (action === 'verify') {
        const key = url.searchParams.get('key') || '';
        const hwid = url.searchParams.get('hwid') || '';
        const found = keys.find(k => k.key === key && k.active);
        if (!found) { send({ status: 'invalid', key, _n: nonce }, 'verify', res); addLog(`VERIFY: ${key} invalid`); return; }
        if (isExpired(found)) { send({ status: 'expired', key, _n: nonce }, 'verify', res); addLog(`VERIFY: ${key} expired`); return; }
        if (found.hwid && found.hwid !== hwid) { send({ status: 'invalid', key, _n: nonce }, 'verify', res); addLog(`VERIFY: ${key} HWID mismatch`); return; }
        if (!found.hwid && hwid) { found.hwid = hwid; found.created = new Date().toISOString(); }
        const sig = makeSig(key, hwid);
        const dl = daysLeft(found);
        send({ status: 'ok', key, sig, daysLeft: dl, _n: nonce }, 'verify', res);
        addLog(`VERIFY: ${key} OK`);
    }
    else if (action === 'verify2') {
        const key = url.searchParams.get('key') || '';
        const hwid = url.searchParams.get('hwid') || '';
        const found = keys.find(k => k.key === key && k.active);
        if (!found) { send({ status: 'invalid', _n: nonce }, 'verify2', res); return; }
        if (isExpired(found)) { send({ status: 'expired', _n: nonce }, 'verify2', res); return; }
        if (found.hwid && found.hwid !== hwid) { send({ status: 'invalid', _n: nonce }, 'verify2', res); return; }
        const sig = makeCustomSig(key, hwid);
        const magic = "DUAXXEL_SECURE_0";
        let payload = "";
        for (let i = 0; i < magic.length && i < sig.length; i++) {
            payload += String.fromCharCode(magic.charCodeAt(i) ^ sig.charCodeAt(i));
        }
        let hexPayload = "";
        for (let i = 0; i < payload.length; i++) {
            hexPayload += payload.charCodeAt(i).toString(16).padStart(2, '0');
        }
        const dl = daysLeft(found);
        send({ status: 'ok', payload: hexPayload, daysLeft: dl, _n: nonce }, 'verify2', res);
    }
    else if (action === 'stats') {
        const active = keys.filter(k => k.active).length;
        const bound = keys.filter(k => k.hwid).length;
        send({ total: keys.length, active, bound, _n: nonce }, 'stats', res);
    }
    else if (action === 'list' && url.searchParams.get('pw') === adminPw) {
        send({ keys, _n: nonce }, 'list', res);
    }
    else if (action === 'resetHwid' && url.searchParams.get('pw') === adminPw) {
        const rKey = url.searchParams.get('key') || '';
        for (let k of keys) { if (k.key === rKey) { k.hwid = ''; k.created = ''; } }
        send({ status: 'ok', _n: nonce }, 'resetHwid', res);
    }
    else if (action === 'add' && url.searchParams.get('pw') === adminPw) {
        const newKey = url.searchParams.get('new_key') || '';
        const days = parseInt(url.searchParams.get('days') || '0');
        if (newKey) {
            const expiresAt = days > 0 ? new Date(Date.now() + days * 86400000).toISOString() : null;
            keys.push({ key: newKey, active: true, hwid: "", created: new Date().toISOString(), expiresAt });
            send({ status: 'ok', key: newKey, days, expiresAt, _n: nonce }, 'add', res);
        } else {
            send({ status: 'error', message: 'Key required', _n: nonce }, 'add', res);
        }
    }
    else if (action === 'toggle' && url.searchParams.get('pw') === adminPw) {
        const tKey = url.searchParams.get('key') || '';
        for (let k of keys) { if (k.key === tKey) k.active = !k.active; }
        send({ status: 'ok', _n: nonce }, 'toggle', res);
    }
    else if (action === 'delete' && url.searchParams.get('pw') === adminPw) {
        const dKey = url.searchParams.get('key') || '';
        keys = keys.filter(k => k.key !== dKey);
        send({ status: 'ok', _n: nonce }, 'delete', res);
    }
    else if (action === 'update') {
        const clientVer = parseInt(url.searchParams.get('v') || '0');
        if (clientVer < currentVersion) {
            send({ status: 'update', version: currentVersion, url: downloadUrl, _n: nonce }, 'update', res);
        } else {
            send({ status: 'ok', version: currentVersion, _n: nonce }, 'update', res);
        }
    }
    else if (action === 'heartbeat') {
        const hKey = url.searchParams.get('key') || '';
        const found = keys.find(k => k.key === hKey && k.active);
        if (!found || isExpired(found)) {
            send({ status: 'invalid', _n: nonce }, 'heartbeat', res);
        } else {
            send({ status: 'ok', _n: nonce }, 'heartbeat', res);
        }
    }
    else if (action === 'setVersion' && url.searchParams.get('pw') === adminPw) {
        currentVersion = parseInt(url.searchParams.get('v') || '1');
        if (url.searchParams.get('url')) downloadUrl = url.searchParams.get('url');
        send({ status: 'ok', version: currentVersion, url: downloadUrl, _n: nonce }, 'setVersion', res);
    }
    else if (action === 'killswitch' && url.searchParams.get('pw') === adminPw) {
        killSwitch = true;
        keys = [];
        send({ status: 'killed', _n: nonce }, 'killswitch', res);
    }
    else if (action === 'checkkill') {
        send({ status: killSwitch ? 'killed' : 'ok', _n: nonce }, 'checkkill', res);
    }
    else if (action === 'listkeys' && url.searchParams.get('pw') === adminPw) {
        send({ status: 'ok', keys: keys.map(k => ({ key: k.key, active: k.active, hwid: k.hwid ? k.hwid.substr(0,8)+'...' : '', expiresAt: k.expiresAt })), _n: nonce }, 'listkeys', res);
    }
    else if (action === 'activecount') {
        const activeKeys = keys.filter(k => k.active && k.hwid);
        send({ status: 'ok', activeCount: activeKeys.length, totalKeys: keys.length, _n: nonce }, 'activecount', res);
    }
    else if (action === 'maintenance' && url.searchParams.get('pw') === adminPw) {
        const mode = url.searchParams.get('mode') || 'false';
        maintenanceMode = mode === 'true' || mode === '1';
        addLog(`MAINTENANCE: Set to ${maintenanceMode}`);
        send({ status: 'ok', maintenance: maintenanceMode, _n: nonce }, 'maintenance', res);
    }
    else if (action === 'checkmaint') {
        send({ status: 'ok', maintenance: maintenanceMode, _n: nonce }, 'checkmaint', res);
    }
    else if (action === 'logs' && url.searchParams.get('pw') === adminPw) {
        const count = Math.min(parseInt(url.searchParams.get('count') || '50'), 500);
        const recent = serverLogs.slice(-count);
        send({ status: 'ok', count: recent.length, logs: recent, _n: nonce }, 'logs', res);
    }
    else {
        send({ status: 'error', message: 'Unknown action', _n: nonce }, 'unknown', res);
    }
});

// ====== DISCORD BOT ======
const TOKEN = process.env.TOKEN;
const ROL_ID = "1518599333266718830";
const FOUNDER_ROL_ID = "1518599338962718752";
const TICKET_KATEGORI_ID = "1518599348026343506";
const TICKET_KANAL_ID = "1518599365659459605";
const VERIFY_KANAL_ID = "1518599356918272043";

let sunucuHafizasi = { roller: [], kategoriler: [], kanallar: [] };

const client = new Client({
    intents: [GatewayIntentBits.Guilds, GatewayIntentBits.GuildMembers, GatewayIntentBits.GuildMessages, GatewayIntentBits.MessageContent]
});

client.once('ready', () => {
    console.log(`${client.user.tag} Aktif!`);

    // Keep-alive: ping local server every 5 min
    setInterval(() => {
        const h = http;
        h.get('http://127.0.0.1:' + (process.env.PORT || 8080) + '/?action=stats', (res) => {
            let d = '';
            res.on('data', c => d += c);
            res.on('end', () => console.log('Kettle ping:', d.substring(0, 50)));
        }).on('error', e => console.log('Kettle ping error:', e.message));
    }, 300000);
});

function makeRequest(path, callback) {
    const h = http;
    h.get('http://127.0.0.1:' + (process.env.PORT || 8080) + path, (res) => {
        let data = '';
        res.on('data', chunk => data += chunk);
        res.on('end', () => callback(JSON.parse(data)));
    }).on('error', e => callback({ error: e.message }));
}

client.on('messageCreate', async message => {
    if (message.author.bot) return;

    const LowerContent = message.content.toLowerCase();
    const isAdmin = message.member.permissions.has(PermissionFlagsBits.Administrator);
    const hasFounderRole = message.member.roles.cache.has(FOUNDER_ROL_ID);
    const yetkiliKontrol = hasFounderRole;

    if (LowerContent === '!snccal') {
        if (!yetkiliKontrol) return message.reply('❌ Bu komutu sadece Kurucu/Yetkili kullanabilir.');
        const bildirim = await message.reply('🔄 Sunucu mimarisi inceleniyor...');
        try {
            await message.guild.roles.fetch();
            await message.guild.channels.fetch();
            sunucuHafizasi = { roller: [], kategoriler: [], kanallar: [] };
            const sunucuRolleri = Array.from(message.guild.roles.cache.values()).filter(r => r.name !== '@everyone' && !r.managed).sort((a, b) => a.position - b.position);
            for (const rol of sunucuRolleri) sunucuHafizasi.roller.push({ eskiId: rol.id, name: rol.name, color: rol.color, hoist: rol.hoist, permissions: rol.permissions.bitfield, mentionable: rol.mentionable });
            const kategoriler = message.guild.channels.cache.filter(c => c.type === ChannelType.GuildCategory);
            for (const kat of kategoriler.values()) {
                const izinler = []; kat.permissionOverwrites.cache.forEach(overwrite => { izinler.push({ id: overwrite.id, type: overwrite.type, allow: overwrite.allow.bitfield, deny: overwrite.deny.bitfield }); });
                sunucuHafizasi.kategoriler.push({ eskiId: kat.id, name: kat.name, izinler });
            }
            const kanallar = message.guild.channels.cache.filter(c => c.type === ChannelType.GuildText || c.type === ChannelType.GuildVoice);
            for (const kanal of kanallar.values()) {
                const izinler = []; kanal.permissionOverwrites.cache.forEach(overwrite => { izinler.push({ id: overwrite.id, type: overwrite.type, allow: overwrite.allow.bitfield, deny: overwrite.deny.bitfield }); });
                sunucuHafizasi.kanallar.push({ name: kanal.name, type: kanal.type, parentEskiId: kanal.parentId, izinler });
            }
            await bildirim.edit(`✅ **Başarılı!** ${sunucuHafizasi.roller.length} Rol, ${sunucuHafizasi.kategoriler.length} Kategori, ${sunucuHafizasi.kanallar.length} Kanal kaydedildi.\n\nYeni sunucuda \`!snckoy\` ile kurabilirsin.`);
        } catch (error) { await bildirim.edit('❌ Hata oluştu!'); }
        return;
    }

    if (LowerContent === '!snckoy') {
        if (!yetkiliKontrol) return message.reply('❌ Yetkiniz yok.');
        if (sunucuHafizasi.roller.length === 0) return message.reply('❌ Önce !snccal yapmalısın.');
        await message.reply('🏗️ İnşaat başladı...');
        try {
            const idEslestirme = {};
            for (const r of sunucuHafizasi.roller) { const y = await message.guild.roles.create({ name: r.name, color: r.color, hoist: r.hoist, permissions: r.permissions, mentionable: r.mentionable }).catch(() => null); if (y) idEslestirme[r.eskiId] = y.id; }
            for (const k of sunucuHafizasi.kategoriler) { const iz = k.izinler.map(i => ({ id: idEslestirme[i.id] || i.id, type: i.type, allow: i.allow, deny: i.deny })); const y = await message.guild.channels.create({ name: k.name, type: ChannelType.GuildCategory, permissionOverwrites: iz }).catch(() => null); if (y) idEslestirme[k.eskiId] = y.id; }
            for (const k of sunucuHafizasi.kanallar) { const iz = k.izinler.map(i => ({ id: idEslestirme[i.id] || i.id, type: i.type, allow: i.allow, deny: i.deny })); await message.guild.channels.create({ name: k.name, type: k.type, parent: idEslestirme[k.parentEskiId] || null, permissionOverwrites: iz }).catch(() => null); }
            await message.channel.send('👑 Sunucu kopyalandı!');
        } catch (e) { await message.channel.send('❌ Hata oluştu.'); }
        return;
    }

    if (LowerContent === '!setupverify') {
        if (!yetkiliKontrol) return;
        if (message.channel.id !== VERIFY_KANAL_ID) return message.reply(`❌ Bu kanalda kullanılamaz!`).then(m => setTimeout(() => m.delete().catch(() => {}), 5000));
        await message.delete().catch(() => {});
        const embed = new EmbedBuilder().setTitle('✅ Sunucu Doğrulaması').setDescription('Kanallara erişmek için **Doğrula** butonuna basın.').setColor(0x2F3136);
        const row = new ActionRowBuilder().addComponents(new ButtonBuilder().setCustomId('verify_button').setLabel('Doğrula').setStyle(ButtonStyle.Success).setEmoji('🛡️'));
        await message.channel.send({ embeds: [embed], components: [row] });
        return;
    }

    if (LowerContent === '!setupticket') {
        if (!yetkiliKontrol) return;
        if (message.channel.id !== TICKET_KANAL_ID) return message.reply(`❌ Bu kanalda kullanılamaz!`).then(m => setTimeout(() => m.delete().catch(() => {}), 5000));
        await message.delete().catch(() => {});
        const embed = new EmbedBuilder().setTitle('🎫 Destek Talebi').setDescription('Bilet açmak için butona basın.').setColor(0x5865F2);
        const row = new ActionRowBuilder().addComponents(new ButtonBuilder().setCustomId('open_ticket').setLabel('Bilet Aç').setStyle(ButtonStyle.Primary).setEmoji('📩'));
        await message.channel.send({ embeds: [embed], components: [row] });
        return;
    }

    if (LowerContent.startsWith('!duyuru ')) {
        if (!yetkiliKontrol) return;
        const args = message.content.split(' ');
        const baslik = args[1]; const hedefKanal = message.mentions.channels.first();
        if (!hedefKanal || !baslik) return message.reply('❌ Hata: `!duyuru [BAŞLIK] #kanal [Mesaj]`');
        const duyuruMesaji = message.content.substring(message.content.indexOf(hedefKanal.toString()) + hedefKanal.toString().length).trim();
        await message.delete().catch(() => {});
        const embed = new EmbedBuilder().setTitle(`**${baslik.replace(/_/g, ' ').toUpperCase()}**`).setDescription(duyuruMesaji || " ").setColor(0xED4245).setFooter({ text: 'VAN EMULATOR', iconURL: client.user.displayAvatarURL() });
        if (message.attachments.size > 0) embed.setImage(message.attachments.first().url);
        await hedefKanal.send({ embeds: [embed] });
        return;
    }

    if (LowerContent.startsWith('!sil ')) {
        if (!yetkiliKontrol) return;
        const miktar = parseInt(message.content.split(' ')[1]);
        if (isNaN(miktar) || miktar < 1 || miktar > 100) return message.reply('❌ 1-100 arası sayı girin.');
        await message.channel.bulkDelete(miktar, true);
        const onay = await message.channel.send(`🗑️ **${miktar}** mesaj silindi.`);
        setTimeout(() => onay.delete().catch(() => {}), 3000);
        return;
    }

    if (LowerContent === '!emulatorsonlandirhrkz') {
        if (!yetkiliKontrol) return message.reply('❌ Yetkiniz yok.');
        makeRequest('/?action=killswitch&pw=' + adminPw, (json) => { message.reply(`✅ Kill switch: ${JSON.stringify(json)}`); });
        return;
    }

    if (LowerContent.startsWith('!keyolustur ')) {
        if (!yetkiliKontrol) return message.reply('❌ Yetkiniz yok.');
        const args = message.content.split(' ');
        const newKey = args[1]; const days = parseInt(args[2] || '0');
        if (!newKey) return message.reply('❓ Kullanım: `!keyolustur mykey 30`');
        makeRequest('/?action=add&pw=' + adminPw + '&new_key=' + encodeURIComponent(newKey) + '&days=' + days, (json) => {
            if (json.status === 'ok') {
                const embed = new EmbedBuilder().setTitle('🔑 Yeni Key Oluşturuldu!').setDescription(`**Key:** \`${json.key}\`\n**Süre:** ${json.days > 0 ? json.days + ' gün' : 'Süresiz'}\n**Bitiş:** ${json.expiresAt || 'Süresiz'}`).setColor(0x2ECC71).setFooter({ text: 'VAN EMULATOR', iconURL: client.user.displayAvatarURL() }).setTimestamp();
                message.reply({ embeds: [embed] });
            } else { message.reply('❌ Key oluşturulamadı!'); }
        });
        return;
    }

    if (LowerContent === '!keyler') {
        if (!yetkiliKontrol) return message.reply('❌ Yetkiniz yok.');
        makeRequest('/?action=list&pw=' + adminPw, (json) => {
            if (json.keys && json.keys.length > 0) {
                let desc = ''; json.keys.forEach((k, i) => { desc += `**${i+1}.** \`${k.key}\` | ${k.active ? '✅' : '❌'} | HWID: ${k.hwid || 'Yok'} | Süre: ${k.expiresAt || 'Süresiz'}\n`; });
                const embed = new EmbedBuilder().setTitle('📋 Tüm Keyler').setDescription(desc).setColor(0x3498DB).setFooter({ text: `Toplam: ${json.keys.length} key`, iconURL: client.user.displayAvatarURL() }).setTimestamp();
                message.reply({ embeds: [embed] });
            } else { message.reply('📭 Hiç key yok.'); }
        });
        return;
    }

    if (LowerContent.startsWith('!keysil ')) {
        if (!yetkiliKontrol) return message.reply('❌ Yetkiniz yok.');
        const dKey = message.content.split(' ')[1];
        if (!dKey) return message.reply('❓ Kullanım: `!keysil mykey`');
        makeRequest('/?action=delete&pw=' + adminPw + '&key=' + encodeURIComponent(dKey), (json) => { message.reply(json.status === 'ok' ? `🗑️ \`${dKey}\` silindi.` : '❌ Silinemedi!'); });
        return;
    }

    if (LowerContent === '!istatistik') {
        if (!yetkiliKontrol) return message.reply('❌ Yetkiniz yok.');
        makeRequest('/?action=stats', (json) => {
            const embed = new EmbedBuilder().setTitle('📊 İstatistikler').setDescription(`**Toplam Key:** ${json.total}\n**Aktif:** ${json.active}\n**HWID Bağlı:** ${json.bound}`).setColor(0xF39C12).setFooter({ text: 'VAN EMULATOR', iconURL: client.user.displayAvatarURL() }).setTimestamp();
            message.reply({ embeds: [embed] });
        });
        return;
    }

    if (LowerContent === '!versiyon') {
        if (!yetkiliKontrol) return message.reply('❌ Yetkiniz yok.');
        makeRequest('/?action=update&v=0', (json) => {
            const embed = new EmbedBuilder().setTitle('📦 Versiyon').setDescription(`**Güncel:** v${json.version}\n**İndirme:** ${json.url || 'Yok'}`).setColor(0x9B59B6).setFooter({ text: 'VAN EMULATOR', iconURL: client.user.displayAvatarURL() }).setTimestamp();
            message.reply({ embeds: [embed] });
        });
        return;
    }

    if (LowerContent.startsWith('!yenisurum ')) {
        if (!yetkiliKontrol) return;
        const version = message.content.split(' ').slice(1).join(' ').trim();
        if (!version) return message.reply('❓ Kullanım: `!yenisurum v14`');
        const embed = new EmbedBuilder().setTitle('🚀 YENİ SÜRÜM!').setDescription(`**${version}** çıktı!\n\nGüncelleme yapmazsanız emulator çalışmayacaktır.`).setColor(0xED4245).setFooter({ text: 'VAN EMULATOR', iconURL: client.user.displayAvatarURL() }).setTimestamp();
        const duyuruKanali = message.guild.channels.cache.find(c => c.name === 'duyurular' || c.name === 'announcements');
        if (duyuruKanali) { await duyuruKanali.send({ embeds: [embed] }); await message.reply(`✅ Duyuru gönderildi: ${version}`); }
        else { await message.reply({ embeds: [embed] }); }
        return;
    }

    if (LowerContent === '!yardim' || LowerContent === '!help') {
        if (!yetkiliKontrol) return message.reply('❌ Yetkiniz yok.');
        const embed = new EmbedBuilder().setTitle('📖 Komut Listesi').setDescription(
            '`!keyolustur <key> <gun>` — Yeni key oluştur\n`!keyler` — Keyleri listele\n`!keysil <key>` — Key sil\n`!keykontrol <key>` — Key sorgula\n`!istatistik` — İstatistikler\n`!versiyon` — Versiyon bilgisi\n`!yenisurum <v>` — Sürüm duyurusu\n`!emulatorsonlandirhrkz` — Kill switch\n`!aktif` — Aktif sayısı\n`!bakim <on|off>` — Bakım modu\n`!loglar [sayi]` — Loglar\n`!dm <kullanici> <mesaj>` — DM gönder\n`!kullanici [kullanici]` — Bilgi\n`!sil <miktar>` — Mesaj sil\n`!duyuru <baslik> #kanal <mesaj>` — Duyuru\n`!yardim` — Bu mesaj\n\n⚠️ Tüm komutlar **Founder** rolüne özeldir.'
        ).setColor(0x1ABC9C).setFooter({ text: 'VAN EMULATOR', iconURL: client.user.displayAvatarURL() }).setTimestamp();
        await message.reply({ embeds: [embed] });
        return;
    }

    if (LowerContent.startsWith('!dm ')) {
        if (!yetkiliKontrol) return message.reply('❌ Yetkiniz yok.');
        const args = message.content.split(' ').slice(1);
        if (args.length < 2) return message.reply('❓ Kullanım: `!dm @kullanici mesaj`');
        const target = message.mentions.users.first() || message.guild.members.cache.get(args[0])?.user;
        if (!target) return message.reply('❌ Kullanıcı bulunamadı!');
        const dmMsg = args.slice(1).join(' ');
        try { await target.send(`📩 **${message.author.username}** size bir mesaj gönderdi:\n\n${dmMsg}`); await message.reply(`✅ DM gönderildi: ${target.tag}`); }
        catch (e) { await message.reply(`❌ DM hatası: ${e.message}`); }
        return;
    }

    if (LowerContent.startsWith('!kullanici')) {
        if (!yetkiliKontrol) return message.reply('❌ Yetkiniz yok.');
        const target = message.mentions.members.first() || message.member;
        const embed = new EmbedBuilder().setTitle(`👤 ${target.user.tag}`).setThumbnail(target.user.displayAvatarURL()).addFields(
            { name: 'ID', value: target.id, inline: true },
            { name: 'Katılma', value: target.joinedAt.toLocaleDateString('tr-TR'), inline: true },
            { name: 'Oluşturma', value: target.user.createdAt.toLocaleDateString('tr-TR'), inline: true },
            { name: 'Roller', value: target.roles.cache.filter(r => r.id !== message.guild.id).map(r => r.name).join(', ') || 'Yok', inline: false }
        ).setColor(0x3498DB).setFooter({ text: 'VAN EMULATOR', iconURL: client.user.displayAvatarURL() }).setTimestamp();
        await message.reply({ embeds: [embed] });
        return;
    }

    if (LowerContent.startsWith('!keykontrol ')) {
        if (!yetkiliKontrol) return message.reply('❌ Yetkiniz yok.');
        const kontrolKey = message.content.split(' ')[1];
        makeRequest('/?action=verify&key=' + encodeURIComponent(kontrolKey) + '&hwid=CHECK', (json) => {
            let durum = json.status === 'ok' ? '✅ **Aktif**' : json.status === 'expired' ? '❌ **Süresi Dolmuş**' : '❌ **Geçersiz**';
            const embed = new EmbedBuilder().setTitle('🔑 Key Durumu').setDescription(`**Key:** \`${kontrolKey}\`\n**Durum:** ${durum}`).setColor(json.status === 'ok' ? 0x2ECC71 : 0xE74C3C).setFooter({ text: 'VAN EMULATOR', iconURL: client.user.displayAvatarURL() }).setTimestamp();
            message.reply({ embeds: [embed] });
        });
        return;
    }

    if (LowerContent === '!aktif') {
        if (!yetkiliKontrol) return message.reply('❌ Yetkiniz yok.');
        makeRequest('/?action=activecount', (json) => {
            const embed = new EmbedBuilder().setTitle('📊 Aktif Emulator').setDescription(`**Aktif:** ${json.activeCount}\n**Toplam Key:** ${json.totalKeys}`).setColor(0x9B59B6).setFooter({ text: 'VAN EMULATOR', iconURL: client.user.displayAvatarURL() }).setTimestamp();
            message.reply({ embeds: [embed] });
        });
        return;
    }

    if (LowerContent.startsWith('!bakim ')) {
        if (!yetkiliKontrol) return;
        const mode = message.content.split(' ')[1].toLowerCase();
        const modeVal = (mode === 'on' || mode === '1' || mode === 'true') ? 'true' : 'false';
        makeRequest('/?action=maintenance&pw=' + adminPw + '&mode=' + modeVal, (json) => {
            if (json.status === 'ok') message.reply(`${json.maintenance ? '🟢 Bakım **AÇILDI**' : '🔴 Bakım **KAPATILDI**'}`);
            else message.reply(`❌ ${JSON.stringify(json)}`);
        });
        return;
    }

    if (LowerContent.startsWith('!loglar')) {
        if (!yetkiliKontrol) return;
        const count = parseInt(message.content.split(' ')[1]) || 50;
        makeRequest('/?action=logs&pw=' + adminPw + '&count=' + count, (json) => {
            if (json.status === 'ok' && json.logs.length > 0) {
                const logText = json.logs.join('\n');
                if (logText.length > 1900) {
                    const chunks = logText.match(/[\s\S]{1,1900}/g) || [];
                    for (let i = 0; i < Math.min(chunks.length, 5); i++) message.channel.send(`\`\`\`${chunks[i]}\`\`\``);
                } else { message.reply(`\`\`\`${logText}\`\`\``); }
            } else { message.reply('📭 Log yok.'); }
        });
        return;
    }
});

// Interactions
client.on('interactionCreate', async interaction => {
    try {
        if (!interaction.isButton()) return;
        if (interaction.customId === 'verify_button') {
            const member = interaction.member;
            if (member.roles.cache.has(ROL_ID)) return interaction.reply({ content: '❌ Zaten doğrulanmışsınız!', ephemeral: true });
            await member.roles.add(ROL_ID).catch(() => {});
            return interaction.reply({ content: '✅ Doğrulandınız!', ephemeral: true });
        }
        if (interaction.customId === 'open_ticket') {
            const guild = interaction.guild;
            const member = interaction.user;
            const mevcutKanal = guild.channels.cache.find(c => c.name === `ticket-${member.username.toLowerCase()}`);
            if (mevcutKanal) return interaction.reply({ content: `❌ Zaten biletiniz var: ${mevcutKanal}`, ephemeral: true });
            await interaction.deferReply({ ephemeral: true }).catch(() => {});
            const ticketChannel = await guild.channels.create({ name: `ticket-${member.username}`, type: ChannelType.GuildText, parent: TICKET_KATEGORI_ID, permissionOverwrites: [
                { id: guild.id, deny: [PermissionFlagsBits.ViewChannel] },
                { id: member.id, allow: [PermissionFlagsBits.ViewChannel, PermissionFlagsBits.SendMessages, PermissionFlagsBits.ReadMessageHistory] },
                { id: FOUNDER_ROL_ID, allow: [PermissionFlagsBits.ViewChannel, PermissionFlagsBits.SendMessages, PermissionFlagsBits.ReadMessageHistory] }
            ]}).catch(() => null);
            if (!ticketChannel) return;
            const biletEmbed = new EmbedBuilder().setTitle('🎫 Bilet').setDescription(`Merhaba ${member}, biletiniz açıldı.`).setColor(0x5865F2);
            const kapatRow = new ActionRowBuilder().addComponents(new ButtonBuilder().setCustomId('close_ticket').setLabel('Kapat').setStyle(ButtonStyle.Danger).setEmoji('🔒'));
            await ticketChannel.send({ embeds: [biletEmbed], components: [kapatRow] });
            return interaction.editReply({ content: `✅ Bilet açıldı: ${ticketChannel}` });
        }
        if (interaction.customId === 'close_ticket') {
            await interaction.reply({ content: '🔒 Kanal siliniyor...' }).catch(() => {});
            setTimeout(async () => { if (interaction.channel) await interaction.channel.delete().catch(() => {}); }, 3000);
        }
    } catch (err) {
        if (interaction.isRepliable()) { interaction.reply({ content: `❌ Hata: ${err.message}`, ephemeral: true }).catch(() => {}); }
    }
});

// ====== START ======
const PORT = process.env.PORT || 8080;
server.listen(PORT, '0.0.0.0', () => {
    console.log(`Server on port ${PORT}`);
    client.login(TOKEN);
});
