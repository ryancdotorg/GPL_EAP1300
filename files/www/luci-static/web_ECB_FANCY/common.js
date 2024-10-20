/***************** Prototype *************************/
if(typeof String.prototype.trim !== 'function')
{
	String.prototype.trim = function()
	{
		return this.replace(/^\s+|\s+$/g, '');
	}
}
/******************************************/
$(function() {
    var moveLeft = 0;
    var moveDown = 0;
    $(".popper").hover(function(e) {
        var target = '#' + ($(this).attr('data-popbox'));
         
        $(target).show();
        moveLeft = $(this).outerWidth();
        moveDown = ($(target).outerHeight() / 2);
    }, function() {
        var target = '#' + ($(this).attr('data-popbox'));
        $(target).hide();
    });
 
    $(".popper").mousemove(function(e) {
        var target = '#' + ($(this).attr('data-popbox'));
         
        leftD = e.pageX + parseInt(moveLeft);
        maxRight = leftD + $(target).outerWidth();
        windowLeft = $(window).width() - 40;
        windowRight = 0;
        maxLeft = e.pageX - (parseInt(moveLeft) + $(target).outerWidth() + 20);
         
        if(maxRight > windowLeft && maxLeft > windowRight)
        {
            leftD = maxLeft;
        }
     
        topD = e.pageY - parseInt(moveDown);
        maxBottom = parseInt(e.pageY + parseInt(moveDown) + 20);
        windowBottom = parseInt(parseInt($(document).scrollTop()) + parseInt($(window).height()));
        maxTop = topD;
        windowTop = parseInt($(document).scrollTop());
        if(maxBottom > windowBottom)
        {
            topD = windowBottom - $(target).outerHeight() - 20;
        } else if(maxTop < windowTop){
            topD = windowTop + 20;
        }
     
        $(target).css('top', topD).css('left', leftD);
    });
});

/********************************************/
function dw(str)
{
	document.write(str);
}
function getById(id)
{
	return document.getElementById(id);
}
function getByName(name)
{
	return document.getElementsByName(name)[0];
}
function getDigit(str, num) {
	i=1;
	if ( num != 1 ) {
		while (i!=num && str.length!=0) {
			if ( str.charAt(0) == '.' ) {
				i++;
			}
			str = str.substring(1);
		}
		if ( i!=num )
			return -1;
	}
	for (i=0; i<str.length; i++) {
		if ( str.charAt(i) == '.' ) {
			str = str.substring(0, i);
			break;
		}
	}
	if ( str.length == 0)
		return -1;
	d = parseInt(str, 10);
	return d;
}
function getBroadcastIP(ip, mask)
{
	var broadcastIP="";
	var ip_t=ip.split(".");
	var mask_t=mask.split(".");
	for(var i=0;i<4;i++){
		broadcastIP=broadcastIP+(ip_t[i] | (255-mask_t[i]));
		if(i!=3)
			broadcastIP=broadcastIP+'.';
	}
	return broadcastIP;
}
function isNumber(n)
{
	return !/[^0-9]/g.test(n)
}
function isBetween(n, min, max)
{
	return ((parseInt(n,10) <= parseInt(max,10)) & (parseInt(n,10) >= parseInt(min,10)))
}
function setfocus(target)
{
	target.value = target.defaultValue;
	target.focus();
}
function isBssid(bssid)
{
	var bssidReg = /^([0-9A-F][0,2,4,6,8,A,C,E][:-])([0-9A-F]{2}[:-]){4}([0-9A-F]{2})$/;
	var bssidReg_Space = /^([0-9A-F][0,2,4,6,8,A,C,E][\s])([0-9A-F]{2}[\s]){4}([0-9A-F]{2})$/;
	var bssidReg_withoutSep = /^([0-9A-F][0,2,4,6,8,A,C,E])([0-9A-F]{2}){4}([0-9A-F]{2})$/;
	return (bssidReg.test(bssid) || bssidReg_Space.test(bssid) || bssidReg_withoutSep.test(bssid));
}
function isSubMask(mask)
{
	var submaskReg = /(^255\.(0|128|192|224|24[08]|25[245])\.0\.0$)|(^255\.255\.(0|128|192|224|24[08]|25[245])\.0$)|(^255\.255\.255\.(0|128|192|224|24[08]|252)$)/;
	return submaskReg.test(mask);
}
/** A valid IP should be calculate by subnet mask, incloud of X.X.X.255 **/
function isValidIpaddr(ip)
{
	var IPRegEx = new RegExp("^(25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]{1}\\d{1}|[1-9])\\.(25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]{1}\\d{1}|\\d)\\.(25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]{1}\\d{1}|\\d)\\.(25[0-5]|2[0-5]\\d|1\\d{2}|[1-9]{1}\\d{1}|[1-9])$");
	if (isSameSubnet("127.0.0.0", "255.0.0.0", ip) || isSameSubnet("169.254.0.0", "255.255.0.0", ip)){
		return false;
	}

	return IPRegEx.test(ip);
}
function isIpaddr(ip)
{
	var IPRegEx = new RegExp("^(25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]{1}\\d{1}|[1-9])\\.(25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]{1}\\d{1}|\\d)\\.(25[0-5]|2[0-4]\\d|1\\d{2}|[1-9]{1}\\d{1}|\\d)\\.(25[0-4]|2[0-4]\\d|1\\d{2}|[1-9]{1}\\d{1}|[1-9])$");
	if (isSameSubnet("127.0.0.0", "255.0.0.0", ip) || isSameSubnet("169.254.0.0", "255.255.0.0", ip)){
		return false;
	}

	return IPRegEx.test(ip);
}
function isIpv6Addr(addr)
{
	reg = /(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))/ ;
	return reg.test(addr);
}
function isDomainName(dns){
	var DOMAIN_NAME_REGX =new RegExp("^((.[a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z]|[A-Za-z][A-Za-z0-9\-]*[A-Za-z0-9])$");
	return DOMAIN_NAME_REGX.test(dns);
}
function isHostName(addr)
{
	var HOST_NAME_REGX =new RegExp("^([a-zA-Z0-9]([a-zA-Z0-9\\-]{0,61}[a-zA-Z0-9])?\\.)+[a-zA-Z]{2,6}$");
	return HOST_NAME_REGX.test(addr);
}
function isEmail(email)
{
	var EMAIL_REGX = /^\w+((-\w+)|(\.\w+))*\@[A-Za-z0-9]+((\.|-)[A-Za-z0-9]+)*\.[A-Za-z]+$/;
	return EMAIL_REGX.test(email);
}
function isBroadcastIp(ip, mask)
{
	mask = mask || "255.255.255.0";
	return (getBroadcastIP(ip, mask) == ip)
}
function isMulticast(ip)
{
	var MULTICAST_IP_REGX = new RegExp("^(22[4-9]|2[3-4][0-9]|25[0-5])\\.(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])\\.(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])\\.(\\d{1,2}|1\\d\\d|2[0-4]\\d|25[0-5])$");
	var MULTICAST_IPv6_REGX = /^FF/i;
	if(isIpv6Addr(ip)){
		return MULTICAST_IPv6_REGX.test(ip);
	}
	return MULTICAST_IP_REGX.test(ip);
}
function hasSpecialChar(str)
{
	var SPECIAL_CHAR_REGX = new RegExp("[\\\\#'\"/:;&[ ]");
	return SPECIAL_CHAR_REGX.test(str);
}
function checkClientRange(start,end)
{
	var start_d, end_d;

	start_d=getDigit(start,4);
	start_d+=getDigit(start,3)*256;
	start_d+=getDigit(start,2)*256*256;
	start_d+=getDigit(start,1)*256*256*256;
	end_d=getDigit(end,4);
	end_d+=getDigit(end,3)*256;
	end_d+=getDigit(end,2)*256*256;
	end_d+=getDigit(end,1)*256*256*256;
	if(start_d<end_d)
		return true;
	return false;
}
function isSameSubnet(ip1,mask1,ip2,mask2)
{
	var ip1_t=ip1.split(".");
	var mask1_t=mask1.split(".");
	var ip2_t=ip2.split(".");
	if(typeof(mask2)!="undefined"){
		var mask2_t=mask2.split(".");
		for(var i=0;i<4;i++)
		{
			if((parseInt(ip1_t[i]) & parseInt(mask1_t[i])) != (parseInt(ip2_t[i]) & parseInt(mask1_t[i]))){
				if((parseInt(ip1_t[i]) & parseInt(mask2_t[i])) != (parseInt(ip2_t[i]) & parseInt(mask2_t[i])))
					return false;
			}
		}
	}
	else{
		for(var i=0;i<4;i++)
		{
			if((parseInt(ip1_t[i]) & parseInt(mask1_t[i])) != (parseInt(ip2_t[i]) & parseInt(mask1_t[i])))
				return false;
		}
	}
	return true;
}
function isSameIpRange(sip1,eip1,sip2,eip2)
{
	var Sip1_t=sip1.split(".");
	var Eip1_t=(eip1)!=""?eip1.split("."):"0";
	var Sip2_t=(sip2)!=""?sip2.split("."):"0";
	var Eip2_t=(eip2)!=""?eip2.split("."):"0";
	var sameCount=0;

	if(sip1==0 && sip2==0)
		return false;
	else if(sip1!=0 && sip2==0)
		return true;

	for(var i=0;i<4;i++){
		if(Eip1_t!="0" && Eip2_t!="0"){
			if((parseInt(Sip1_t[i])>=parseInt(Sip2_t[i]) && parseInt(Sip1_t[i])<=parseInt(Eip2_t[i])) || 
			(parseInt(Eip1_t[i])>=parseInt(Sip2_t[i]) && parseInt(Eip1_t[i])<=parseInt(Eip2_t[i])) || 
			(parseInt(Sip1_t[i])<=parseInt(Sip2_t[i]) && parseInt(Eip1_t[i])>=parseInt(Eip2_t[i])))
				sameCount++;
		}
		else if(Eip1_t!="0" && Eip2_t=="0"){
			if((parseInt(Sip1_t[i])<=parseInt(Sip2_t[i])) && (parseInt(Eip1_t[i])>=parseInt(Sip2_t[i])))
				sameCount++;
		}
		else if(Eip1_t=="0" && Eip2_t!="0"){
			if((parseInt(Sip1_t[i])>=parseInt(Sip2_t[i])) && (parseInt(Sip1_t[i])<=parseInt(Eip2_t[i])))
				sameCount++;
		}
		else if(Eip1_t=="0" && Eip2_t=="0"){
			if(parseInt(Sip1_t[i])==parseInt(Sip2_t[i]))
				sameCount++;
		}
	}
	if(sameCount==4)
		return true;
	else
		return false;
}
function isLinklocal(ip)
{
	return isSameSubnet("169.254.0.1", "255.255.0.0", ip);
}
function isLoopback(ip)
{
	return (ip == "127.0.0.1")
}
function isInvalid(ip)
{
	return (ip == "0.0.0.0")
}
function htmlspecialchars(str)
{
	str = str || "";
	if(encodeURIComponent)
	{
		str = encodeURIComponent(str);
	}
	else if(escape)
	{
		str = escape(str);
	}
	else
	{
		str = str.replace(/\"/gi, '%22');
		str = str.replace("?", '%3F');
		str = str.replace("#", '%23');
		str = str.replace("&", '%26');
		str = str.replace(";", '%3B');
		str = str.replace("+", '%2B');
	}
	return str;
}

function showWMode(m)
{
	var mode = {"11ng":"n/g/b", "11bg":"g/b", "11b":"b", "11g":"g", "11n":"n", "11a":"a", "11ac":"ac/n/a", "11na":"n/a", "11axg":"ax/n/g/b", "11axa":"ax/ac/n/a"};
	return "802.11 " + mode[m];
}
function showStaHWMode(m)
{
    //sync with qca-wifi_x.x/src/os/linux/src/ieee80211_wireless.c: int ieee80211_ioctl_giwname()
	var mode = {"802.11ng":"n/g/b", "802.11g":"g/b", "802.11b":"b", "only":"g", "802.11n":"n", "802.11a":"a", "802.11ac":"ac/n", "802.11na":"n/a", "802.11axg":"ax", "802.11axa":"ax", "802.11":""};
	return "802.11 " + mode[m];
}
function showOpmode(s)
{
	var mode = {"sta":"Client Bridge", "wds_sta":"WDS Station", "ap":"Access Point", "wds_ap":"WDS Access Point", "wds_bridge":"WDS Bridge", "sta_ap":"Repeater"};
	return mode[s];
}
function showBandwidth(b, disp)
{
	disp = disp || "";
	var band = {"HT20_40":((disp == "wireless_setting")?"20/40":"20-40"), "HT20":"20", "HT40":"40", "HT80":"80"};
	return band[b] + " MHz";
}
function isFullwidth(value)
{
	var PatternRegEx = /[^\x00-\xff]+/;
	return PatternRegEx.test(value);
}
function isChinese(value)
{
	var PatternRegEx = /[\u4E00-\u9FA5]+/;
	return PatternRegEx.test(value);
}
function isPattern1(value)
{
	var PatternRegEx = /^((?![\:\;]).)*$/;
	return PatternRegEx.test(value);
}
function isPattern2(value)
{
	var PatternRegEx = /^((?![\:\;\s\[\]\`\#\&\\\/\'\"]).)*$/;
	return PatternRegEx.test(value);
}
function isPattern2_1(value)
{
	var PatternRegEx = /^((?![\:\;\s\[\]\`\#\&\\\/\'\"\,]).)*$/;
	return PatternRegEx.test(value);
}
function isPattern3(value)
{
	var PatternRegEx = /^((?![\:\s\[\]\`\#\&\\\/\'\"]).)*$/;
	return PatternRegEx.test(value);
}
/* Replace pattern to special character for javascript. Remeber to add element to function repSpec of util.lua if add new pattern. */
function repSpec(value)
{
	var escapePat = {1:"repPat1", 2:"repPat2", 3:"repPat3", 4:"repPat4", 5:"repPat5", 6:"repPat6"}, replacePat={1:"\<",2:"\>",3:"\ ",4:"\"",5:"\'",6:"\&"};
	for (key in escapePat) {
		var myRegExp = new RegExp(escapePat[key], 'g');
		value = value.replace(myRegExp,replacePat[key]);
	}
	return value
}
/* Replace pattern to special character for HTML. Remeber to add element to function repSpec of util.lua if add new pattern. */
function repSpecHTML(value,opt)
{
	var escapePat = {1:"repPat1", 2:"repPat2", 3:"repPat3", 4:"repPat4", 5:"repPat5", 6:"repPat6"}
	var replacePat={1:"&#60;",2:"&#62;",3:"&#160;",4:"&#34;",5:"&#39;",6:"&#38;"};
	if (opt==1){
		replacePat[3]="&#32;" /* &#32; is the classic space, &nbsp; and &#160; represents the non-breaking space */
	}
	for (key in escapePat) {
		var myRegExp = new RegExp(escapePat[key], 'g');
		value = value.replace(myRegExp,replacePat[key]);
	}
	return value
}
/* Replace special character for HTML */
function SpecHTML(value)
{
	var escapePat = {1:"\<",2:"\>",3:"\ ",4:"\"",5:"\'"}, replacePat={1:"&#60;",2:"&#62;",3:"&#160;",4:"&#34;",5:"&#39;"};
	for (key in escapePat) {
		var myRegExp = new RegExp(escapePat[key], 'g');
		value = value.replace(myRegExp,replacePat[key]);
	}
	return value
}

