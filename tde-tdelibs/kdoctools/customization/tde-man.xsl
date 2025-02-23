<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:import href="../docbook/xsl/manpages/docbook.xsl"/>

<!-- Disable the automatically generated AUTHORS section by overloading the
  respective template and making it a no-op. -->
<xsl:template match="articleinfo|bookinfo|refentryinfo" mode="authorsect"/>

<!--
  Copied from ../docbook/xsl/manpages/docbook.xsl and changed it so that the
  output file's name is harccoded to 'manpage.troff', and the quiet parameter
  is set to 1 to avoid the 'Writing foo.X' message which write.text.chunk
  prints.
  -->
<xsl:template match="refentry">

  <xsl:variable name="section">
    <xsl:choose>
      <xsl:when test="refmeta/manvolnum">
        <xsl:value-of select="refmeta/manvolnum[1]"/>
      </xsl:when>
      <xsl:when test=".//funcsynopsis">3</xsl:when>
      <xsl:otherwise>1</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>

  <xsl:variable name="name" select="refnamediv/refname[1]"/>

  <!-- standard man page width is 64 chars; 6 chars needed for the two
       (x) volume numbers, and 2 spaces, leaves 56 -->
  <xsl:variable name="twidth" select="(56 - string-length(refmeta/refentrytitle)) div 2"/>

  <xsl:variable name="reftitle" 
		select="substring(refmeta/refentrytitle, 1, $twidth)"/>

  <xsl:variable name="title">
    <xsl:choose>
      <xsl:when test="refentryinfo/title">
        <xsl:value-of select="refentryinfo/title"/>
      </xsl:when>
      <xsl:when test="../referenceinfo/title">
        <xsl:value-of select="../referenceinfo/title"/>
      </xsl:when>
    </xsl:choose>
  </xsl:variable>

  <xsl:variable name="date">
    <xsl:choose>
      <xsl:when test="refentryinfo/date">
        <xsl:value-of select="refentryinfo/date"/>
      </xsl:when>
      <xsl:when test="../referenceinfo/date">
        <xsl:value-of select="../referenceinfo/date"/>
      </xsl:when>
    </xsl:choose>
  </xsl:variable>

  <xsl:variable name="productname">
    <xsl:choose>
      <xsl:when test="refentryinfo/productname">
        <xsl:value-of select="refentryinfo/productname"/>
      </xsl:when>
      <xsl:when test="../referenceinfo/productname">
        <xsl:value-of select="../referenceinfo/productname"/>
      </xsl:when>
    </xsl:choose>
  </xsl:variable>

  <xsl:variable name="filename">
    <xsl:text>manpage.troff</xsl:text>
  </xsl:variable>

  <xsl:call-template name="write.text.chunk">
    <xsl:with-param name="quiet" select="1"/>
    <xsl:with-param name="filename" select="$filename"/>
    <xsl:with-param name="content">
      <xsl:text>.\"Generated by db2man.xsl. Don't modify this, modify the source.
.de Sh \" Subsection
.br
.if t .Sp
.ne 5
.PP
\fB\\$1\fR
.PP
..
.de Sp \" Vertical space (when we can't use .PP)
.if t .sp .5v
.if n .sp
..
.de Ip \" List item
.br
.ie \\n(.$>=3 .ne \\$3
.el .ne 3
.IP "\\$1" \\$2
..
.TH "</xsl:text>
      <xsl:value-of select="translate($reftitle,'abcdefghijklmnopqrstuvwxyz', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ')"/>
      <xsl:text>" </xsl:text>
      <xsl:value-of select="$section"/>
      <xsl:text> "</xsl:text>
      <xsl:value-of select="normalize-space($date)"/>
      <xsl:text>" "</xsl:text>
      <xsl:value-of select="normalize-space($productname)"/>
      <xsl:text>" "</xsl:text>
      <xsl:value-of select="$title"/>
      <xsl:text>"
</xsl:text>
      <xsl:apply-templates/>
      <xsl:text>&#10;</xsl:text>

      <!-- Author section -->
      <xsl:choose>
        <xsl:when test="refentryinfo//author">
          <xsl:apply-templates select="refentryinfo" mode="authorsect"/>
        </xsl:when>
        <xsl:when test="/book/bookinfo//author">
          <xsl:apply-templates select="/book/bookinfo" mode="authorsect"/>
        </xsl:when>
        <xsl:when test="/article/articleinfo//author">
          <xsl:apply-templates select="/article/articleinfo" mode="authorsect"/>
        </xsl:when>
      </xsl:choose>

    </xsl:with-param>
  </xsl:call-template>
  <!-- Now generate stub include pages for every page documented in
       this refentry (except the page itself) -->
  <xsl:for-each select="refnamediv/refname">
    <xsl:if test=". != $name">
      <xsl:call-template name="write.text.chunk">
	<xsl:with-param name="filename"
		        select="concat(normalize-space(.), '.', $section)"/>
	<xsl:with-param name="content" select="concat('.so man',
	      $section, '/', $name, '.', $section, '&#10;')"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:for-each>
</xsl:template>

</xsl:stylesheet>

