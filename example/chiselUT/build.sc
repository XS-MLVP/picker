
import mill._, scalalib._
import coursier.maven.MavenRepository
import mill.scalalib.TestModule._
import scala.sys.process._


object ut extends ScalaModule with ScalaTest{
    override def millSourcePath = os.pwd

    def scalaVersion = "2.13.15"

    def compileClasspath = T {
      super.compileClasspath() ++ unmanagedClasspath()
    }

    def unmanagedClasspath = T {
      val jarPath: String = "picker --show_xcom_lib_location_scala".!!.trim
      val jarFile = os.Path(jarPath)
      Agg(
        PathRef(jarFile),
      )
    }

    override def ivyDeps = Agg(
      ivy"org.chipsalliance::chisel:6.6.0",
      ivy"org.scalatest::scalatest:3.2.19",
    )

    override def scalacPluginIvyDeps = Agg(
      ivy"org.chipsalliance:::chisel-plugin:6.6.0",
    )

    def testFrameworks = Seq("org.scalatest.tools.Framework")
}
